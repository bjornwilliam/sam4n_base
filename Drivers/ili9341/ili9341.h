#ifndef ILI9341_H_
#define ILI9341_H_

// Bit which is sent along with data to let the ili9341 know 
// that the incoming byte is data or parameter and not a command
#define DATA_BIT (1<<9) 

#define ILI9341_TFTWIDTH	240
#define ILI9341_TFTHEIGHT	320

void ili9341_init();
void ili9341_enter_standby();

void ili9341_drawPixel(int16_t x, int16_t y, uint16_t color);

#endif /* ILI9341_H_ */