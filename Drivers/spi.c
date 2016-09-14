#include "spi.h"
#include "pmc.h"
#include "pio.h"

static void (*callBackFunctionPointer)(void); // Make a function pointer so that you can assign callback functions to it

SemaphoreHandle_t spi_handlerIsDoneSempahore = NULL;
SemaphoreHandle_t spi_mutex = NULL;

static void spi_tranceive(uint32_t *transmit_buffer, uint8_t buffer_length, uint32_t *receive_buffer ) {
	SPI->SPI_CR = 1 << 0;
	SPI->SPI_TPR = transmit_buffer; // Give address to tdr register
	SPI->SPI_TCR = buffer_length;  // Give it length of transmit buffer
	SPI->SPI_RPR = receive_buffer; // Give address to rpr register
	SPI->SPI_RCR = buffer_length; // Give it length of receive buffer
	NVIC_EnableIRQ(SPI_IRQn);
}

void spi_freeRTOSTranceive(uint32_t  *transmit_buffer, uint8_t buffer_length, void (*callBackFunc)(void), uint32_t *receive_buffer ) {
	//Aquire the spi resource
	xSemaphoreTake(spi_mutex,portMAX_DELAY);
	callBackFunctionPointer = callBackFunc;
	spi_tranceive(transmit_buffer, buffer_length, receive_buffer);
	// Wait for the SPI_Handler to run ,and signal that the transfer is complete
	xSemaphoreTake(spi_handlerIsDoneSempahore, portMAX_DELAY);
	
	// Release spi resource
	xSemaphoreGive(spi_mutex);
}

void SPI_Handler(void) {
	long lHigherPriorityTaskWoken = pdFALSE;
	SPI->SPI_SR; // MUST READ SR TO CLEAR NSSR
	if (callBackFunctionPointer != NULL) {
		callBackFunctionPointer();
	}
	if (spi_handlerIsDoneSempahore != NULL) {
		xSemaphoreGiveFromISR(spi_handlerIsDoneSempahore,&lHigherPriorityTaskWoken);
	}
	portEND_SWITCHING_ISR(lHigherPriorityTaskWoken);
	SPI->SPI_CR = 1 << 1;
	NVIC_DisableIRQ(SPI_IRQn);
	
}

uint32_t spi_word(bool last_xfer,uint8_t chip_select, uint16_t data) {
	uint32_t chip_select_correct_format;
	switch(chip_select) {
		case 0:
		chip_select_correct_format = 0;
		break;
		case 1:
		chip_select_correct_format = 1 << 16;
		break;
		case 2:
		chip_select_correct_format = 3 << 16;
		break;
		case 3:
		chip_select_correct_format = 7 << 16;
		break;
	}
	if (last_xfer) {
		return ( (1<<24) | chip_select_correct_format | data);
	}
	else {
		return (chip_select_correct_format | data);
	}
}


void spi_setBaudRateHz(uint32_t peripheral_clock_hz, uint32_t baud_rate_hz, uint8_t chip_select) {
	
	if ((peripheral_clock_hz/baud_rate_hz > 255) || ((peripheral_clock_hz/baud_rate_hz) < 1)) {
		while(1);
	}
	else {
		SPI->SPI_CSR[chip_select] |= (peripheral_clock_hz/baud_rate_hz)  << 8;
	}
}


static void spi_setupMux(struct SpiMaster spi_settings) {
	//Assigning SPI pin to correct peripheral
	pio_setMux(PIOA, 14, A);//SPCK pin
	pio_setMux(PIOA, 13, A);//MOSI pin
	pio_setMux(PIOA, 12, A);//MISO pin
	//NPCS0 pin
	switch(spi_settings.cs_0){
		case PA11:
		pio_setMux(PIOA, 11, A);
		break;
		default:
		break;
	}
	//NPCS1 pin
	switch(spi_settings.cs_1) {
		case PA9:
		pio_setMux(PIOA, 9, B);
		break;
		case PA31:
		pio_setMux(PIOA, 31, A);
		break;
		case PB14:
		pio_setMux(PIOB, 14, A);
		break;
		case PC4:
		pio_setMux(PIOC, 4, B);
		break;
		default:
		break;
	}
	//NPCS2 pin
	switch(spi_settings.cs_2){
		case PA10:
		pio_setMux(PIOA, 10, B);
		break;
		case PA30:
		pio_setMux(PIOA, 30, B);
		break;
		case PB2:
		pio_setMux(PIOB, 2, B);
		break;
		default:
		break;
	}
	//NPCS3 pin
	switch(spi_settings.cs_3){
		case PA3:
		pio_setMux(PIOA, 3, B);
		break;
		case PA5:
		pio_setMux(PIOA, 5, B);
		break;
		case PA22:
		pio_setMux(PIOA, 22, B);
		break;
		default:
		break;
	}
}

void spi_masterInit(struct SpiMaster SpiSettings) {
	NVIC_DisableIRQ(SPI_IRQn);
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_SetPriority(SPI_IRQn,SpiSettings.NVIC_spi_interrupt_priority);
	NVIC_EnableIRQ(SPI_IRQn);
	
	pmc_enable_periph_clk(SPI_IRQn); // Enable Spi clock
	spi_setupMux(SpiSettings);
	
	SPI->SPI_CR |= SPI_CR_SPIEN; // Enable SPI
	SPI->SPI_MR |= SPI_MR_MSTR; // Set Master Mode
	SPI->SPI_MR &= ~(1<<2); // Disable Chip select decode
	SPI->SPI_MR |= (1<<1); // Variable peripheral select
	SPI->SPI_MR &= ~(1<<4); // Disable mode fault detection
	

	SPI->SPI_IER  |= 1<<9;  // TXEMPTY Interrupt
	SPI->SPI_PTCR |= 1<<8; // Enable PDC transmit
	SPI->SPI_PTCR |= 1<<0; // Enable PDC receive
}
void spi_chipSelectInit(struct SpiSlaveSettings SpiCsSettings) {
	spi_setBaudRateHz(SpiCsSettings.peripheral_clock_hz,SpiCsSettings.spi_baudRate_hz,SpiCsSettings.chip_select);
	switch (SpiCsSettings.spi_mode) {
		case MODE_0:
		SPI->SPI_CSR[SpiCsSettings.chip_select] &= ~(1<<0);
		SPI->SPI_CSR[SpiCsSettings.chip_select] |= (1<<1);
		break;
		case MODE_1:
		SPI->SPI_CSR[SpiCsSettings.chip_select] &= ~(1<<0);
		SPI->SPI_CSR[SpiCsSettings.chip_select] &=  ~(1<<1);
		break;
		case MODE_2:
		SPI->SPI_CSR[SpiCsSettings.chip_select] |= (1<<0);
		SPI->SPI_CSR[SpiCsSettings.chip_select] |= (1<<1);
		break;
		case MODE_3:
		SPI->SPI_CSR[SpiCsSettings.chip_select] |= (1<<0);
		SPI->SPI_CSR[SpiCsSettings.chip_select] &= ~(1<<1);
		break;
	}
	SPI->SPI_CSR[SpiCsSettings.chip_select] |= (SpiCsSettings.bits_per_transfer-8) << 4;
	SPI->SPI_CSR[SpiCsSettings.chip_select] |= SpiCsSettings.time_until_first_valid_SPCK << 16;
	SPI->SPI_CSR[SpiCsSettings.chip_select] |= SpiCsSettings.delay_between_two_consecutive_transfers << 24;
}