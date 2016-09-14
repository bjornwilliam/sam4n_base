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
	uint32_t transmit_buffer[1] = { spi_word(true, ILI9341_CHIP_SELECT, data) };
	uint32_t receive_buffer[1];
	spi_freeRTOSTranceive(transmit_buffer, 1, 0, receive_buffer);
}

static void ili9341_send_command(uint32_t command) {
	ili9341_select_command_mode();
	ili9341_send_byte(command);
}


static void ili9341_exit_standby() {
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
void ili9341_init(void)
{
	/* Reset the display */
	ili9341_reset_display();

	/* Send commands to exit standby mode */
	ili9341_exit_standby();
}



void ili9341_drawPixel(int16_t x, int16_t y, uint16_t color) {

 	if ((x < 0) ||(x >= ILI9341_TFTWIDTH) || (y < 0) || (y >= ILI9341_TFTHEIGHT)) return;
	 
	uint32_t tbuffer[50];
	uint32_t rbuffer[50];
 	uint32_t current_index =	setAddress(0, tbuffer, x, y, x, y);
	tbuffer[current_index] =	spi_word(false,ILI9341_CHIP_SELECT, ILI9341_CMD_MEMORY_WRITE);
	tbuffer[current_index+1] =  spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color >> 8)));
	tbuffer[current_index+2] =  spi_word(false,ILI9341_CHIP_SELECT, (DATA_BIT | (color & 0xFF)));
	uint8_t transmit_length = current_index + 1;
	spi_freeRTOSTranceive(tbuffer,transmit_length,0,rbuffer);
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
