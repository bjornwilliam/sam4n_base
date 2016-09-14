#include <sam.h>
#include "ili9341.h"
#include "ili9341_regs.h"
#include "ili9341_pioInterface.h"

#include "../delay.h"
#include "../spi.h"

// Default setting is to send MSB first
// BASE LEVEL COMMUNICATION
static void ili9341_select_command_mode();
static void ili9341_select_data_mode();
static void ili9341_send_byte(uint32_t data);
static void ili9341_send_command(uint32_t command);

// 
static uint32_t setAddress(uint32_t start_index, uint32_t *tbuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#define MAX_ILI9341_PACKAGE_SIZE 500
static uint32_t dma_transmit_buffer[MAX_ILI9341_PACKAGE_SIZE];
static uint32_t dma_receive_buffer[MAX_ILI9341_PACKAGE_SIZE];


static void ili9341_reset_display() {
	pio_enableOutput(ILI9341_RESET_PIO, ILI9341_RESET_PIN);
	
	pio_setOutput(ILI9341_RESET_PIO, ILI9341_RESET_PIN, PIN_HIGH);
	delay_ms(10);
	pio_setOutput(ILI9341_RESET_PIO, ILI9341_RESET_PIN, PIN_LOW);
	delay_ms(10);
	pio_setOutput(ILI9341_RESET_PIO, ILI9341_RESET_PIN, PIN_HIGH);
	delay_ms(150);
}

static void ili9341_select_command_mode() {
	pio_enableOutput(ILI9341_DATA_OR_CMD_PIO, ILI9341_DATA_OR_CMD_PIN);
	
	pio_setOutput(ILI9341_DATA_OR_CMD_PIO, ILI9341_DATA_OR_CMD_PIN, PIN_LOW);
}
static void ili9341_select_data_mode() {
	pio_enableOutput(ILI9341_DATA_OR_CMD_PIO, ILI9341_DATA_OR_CMD_PIN);
	
	pio_setOutput(ILI9341_DATA_OR_CMD_PIO, ILI9341_DATA_OR_CMD_PIN, PIN_HIGH);
}

static void ili9341_send_byte(uint32_t data) {
	dma_transmit_buffer[0] = spi_word(true, ILI9341_CHIP_SELECT, data) ;
	spi_freeRTOSTranceive(dma_transmit_buffer, 1, 0, dma_receive_buffer);
}

static void ili9341_send_command(uint32_t command) {
	dma_transmit_buffer[0] = spi_word(true, ILI9341_CHIP_SELECT, command);
	spi_freeRTOSTranceive(dma_transmit_buffer, 1, 0, dma_receive_buffer);
}


void ili9341_exit_standby() {
	ili9341_send_command(ILI9341_CMD_SLEEP_OUT);
	delay_ms(150);
	ili9341_send_command(ILI9341_CMD_DISPLAY_ON);
}

void ili9341_enter_standby() {
	ili9341_send_command(ILI9341_CMD_DISPLAY_OFF);
	delay_ms(150);
	ili9341_send_command(ILI9341_CMD_ENTER_SLEEP_MODE);
}
/**
 * \brief Initialize the controller
 *
 * Used to initialize the ILI9341 display controller by setting up the hardware
 * interface, and setting up the controller according to the manufacturer's
 * description. It also set up the screen orientation to the default state
 * (portrait).
 */
static const uint8_t init_commands[] = {
	4, 0xEF, 0x03, 0x80, 0x02,
	4, 0xCF, 0x00, 0XC1, 0X30,
	5, 0xED, 0x64, 0x03, 0X12, 0X81,
	4, 0xE8, 0x85, 0x00, 0x78,
	6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02,
	2, 0xF7, 0x20,
	3, 0xEA, 0x00, 0x00,
	2, ILI9341_CMD_POWER_CONTROL_1, 0x23, // Power control
	2, ILI9341_CMD_POWER_CONTROL_2, 0x10, // Power control
	3, ILI9341_CMD_VCOM_CONTROL_1, 0x3e, 0x28, // VCM control
	2, ILI9341_CMD_VCOM_CONTROL_2, 0x86, // VCM control2
	2, ILI9341_CMD_MEMORY_ACCESS_CONTROL, 0x48, // Memory Access Control
	2, ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET, 0x55,
	3, ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL, 0x00, 0x18,
	4, ILI9341_CMD_DISPLAY_FUNCTION_CONTROL, 0x08, 0x82, 0x27, // Display Function Control
	2, 0xF2, 0x00, // Gamma Function Disable
	2, ILI9341_CMD_GAMMA_SET, 0x01, // Gamma curve selected
	16, ILI9341_CMD_POSITIVE_GAMMA_CORRECTION, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
		0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00, // Set Gamma
	16, ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
		0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F, // Set Gamma
	0
};
// 	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
// 	const uint8_t *addr = init_commands;
// 	while (1) {
// 		uint8_t count = *addr++;
// 		if (count-- == 0) break;
// 		writecommand_cont(*addr++);
// 		while (count-- > 0) {
// 			writedata8_cont(*addr++);
// 		}
// 	}
// 	writecommand_last(ILI9341_SLPOUT);    // Exit Sleep
// 	SPI.endTransaction();
// 
// 	delay(120); 		
// 	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
// 	writecommand_last(ILI9341_DISPON);    // Display on
// 	SPI.endTransaction();
void ili9341_init(void)
{
	/* Reset the display */
	ili9341_reset_display();
	const uint8_t *addr = init_commands;
	uint32_t dma_index = 0;
	while (1) {
		uint8_t count = *addr++;
		if (count-- == 0) {
			spi_freeRTOSTranceive(dma_transmit_buffer, (dma_index +1), NULL, dma_receive_buffer);
			break;
		}
		dma_transmit_buffer[dma_index] = spi_word(false, ILI9341_CHIP_SELECT, *addr++);
		++dma_index;
		//writecommand_cont(*addr++);
		while (count-- > 0) {
			dma_transmit_buffer[dma_index] = spi_word(false, ILI9341_CHIP_SELECT, (DATA_BIT |(*addr++)));
			++dma_index;
			//writedata8_cont(*addr++);
			
		}
	}
	ili9341_send_command(ILI9341_CMD_SLEEP_OUT);
	delay_ms(150);
	ili9341_send_command(ILI9341_CMD_DISPLAY_ON);
}



void ili9341_drawPixel(int16_t x, int16_t y, uint16_t color) {

 	if ((x < 0) ||(x >= ILI9341_TFTWIDTH) || (y < 0) || (y >= ILI9341_TFTHEIGHT)) return;
	 
 	uint32_t current_index =	setAddress(0, dma_transmit_buffer, x, y, x, y);
	dma_transmit_buffer[current_index] =	spi_word(false,ILI9341_CHIP_SELECT, ILI9341_CMD_MEMORY_WRITE);
	dma_transmit_buffer[current_index+1] =  spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color >> 8)));
	dma_transmit_buffer[current_index+2] =  spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color & 0xFF)));
	uint32_t transmit_length = current_index + 1;
	spi_freeRTOSTranceive(dma_transmit_buffer,transmit_length,0,dma_receive_buffer);
 }


void ili9341_drawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
	if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT)) {
		return;
	}
	if ((y+h-1) >= ILI9341_TFTHEIGHT) {
		h = ILI9341_TFTHEIGHT-y;
	}
	uint32_t current_index = setAddress(0, dma_transmit_buffer, x,y,x,y+h-1);
	dma_transmit_buffer[current_index] = spi_word(false,ILI9341_CHIP_SELECT, ILI9341_CMD_MEMORY_WRITE);
	while (h-- >= 1) {
		current_index = current_index + 2;
		dma_transmit_buffer[current_index-1] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color >> 8)));
		dma_transmit_buffer[current_index] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color & 0xFF)));
	}
	spi_freeRTOSTranceive(dma_transmit_buffer, (current_index + 1), NULL, dma_receive_buffer);
}

static uint32_t setAddress(uint32_t start_index, uint32_t *tbuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	tbuffer[start_index]   = spi_word(false,ILI9341_CHIP_SELECT, ILI9341_CMD_COLUMN_ADDRESS_SET);
	tbuffer[start_index+1] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (x0 >> 8)));
	tbuffer[start_index+2] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (x0 & 0xFF)));
	tbuffer[start_index+3] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (x1 >> 8)));
	tbuffer[start_index+4] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (x1 & 0xFF)));
	
	tbuffer[start_index+5] = spi_word(false,ILI9341_CHIP_SELECT, ILI9341_CMD_PAGE_ADDRESS_SET);
	tbuffer[start_index+6] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (y0 >> 8)));
	tbuffer[start_index+7] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (y0 & 0xFF)));
	tbuffer[start_index+8] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (y1 >> 8)));
	tbuffer[start_index+9] = spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (y1 & 0xFF)));
	
	return (start_index + 10);
}
