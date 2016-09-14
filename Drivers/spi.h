#ifndef SPI_H_
#define SPI_H_

#include "sam.h"
#include "../FreeRTOS/include/FreeRTOS.h"
#include "../FreeRTOS/include/semphr.h"

#include <stdbool.h>

extern SemaphoreHandle_t spi_handlerIsDoneSempahore;
extern SemaphoreHandle_t spi_mutex;

enum SpiMode{
	MODE_0, // CPOL 0 , NCPHA 1
	MODE_1, // CPOL 0 , NCPHA 0
	MODE_2, // CPOL 1 , NCPHA 1
	MODE_3  // CPOL 1 , NCPHA 0
};

enum NCPS0pin{none0, PA11};
enum NCPS1pin{none1, PA9,	PA31,	PB14,	PC4};
enum NCPS2pin{none2, PA10,	PA30,	PB2};
enum NCPS3pin{none3, PA3,	PA5,	PA22};
	
	
enum SpiChipSelect{
	NPCS0 = 0,
	NPCS1,
	NPCS2,
	NPCS3,
};

struct SpiMaster {
	uint8_t NVIC_spi_interrupt_priority; // must be >= 5
	enum NCPS0pin cs_0;
	enum NCPS1pin cs_1;
	enum NCPS2pin cs_2;
	enum NCPS3pin cs_3;
};
struct SpiSlaveSettings {
	enum SpiChipSelect chip_select;
	uint32_t peripheral_clock_hz;
	/*Specify Mode 0..3. Specified in datasheet at 35.7.2 */
	enum SpiMode spi_mode;
	/*Baud rate in Hz for all chip selects*/
	uint32_t spi_baudRate_hz;
	/*Bits per transfer for each chip select. 8 = 8 bits, 9 = 9 bits... 16 = 16 bits. */
	uint8_t bits_per_transfer;
	/* Time  after chip select activated until the first valid SPCK transition.*/
	uint8_t time_until_first_valid_SPCK;
	/* Delay  between two consecutive transfers on the same peripheral without removing the chip select.
	If DLYBCT = 0, no delay is inserted. Look at page 818 for more information*/
	uint8_t delay_between_two_consecutive_transfers;
	};
	
void spi_masterInit(struct SpiMaster SpiSettings );
void spi_chipSelectInit(struct SpiSlaveSettings SpiCsSettings);


void spi_freeRTOSTranceive(uint32_t  *transmit_buffer, uint8_t buffer_length, void (*callBackFunc)(void), uint32_t *receive_buffer);
uint32_t spi_word(bool last_xfer, uint8_t chip_select, uint16_t data);

void spi_setBaudRateHz(uint32_t peripheral_clock_hz, uint32_t baud_rate_hz, uint8_t chip_select);


/*
FUNDAMENTALS:

Setting the bit length of the spi register: 
If you want to push out for example 16 bits on one spi transfer 
you would set "bits_per_transfer[Chipselect] = 16".


Initializing the spi:

Here is an example of how to initialize the spi if you are only using chip select 0 on PA11.
	struct SpiMaster SpiMasterSettings = {
		.NVIC_spi_interrupt_priority = 10,
		.cs_0 = PA11,
		.cs_1 = none1,
		.cs_2 = none2,
		.cs_3 = none3
	};
	struct SpiDevice Spi0 = {
		.chip_select = 0,
		.bits_per_transfer = 8,
		.delay_between_two_consecutive_transfers = 0,
		.peripheral_clock_hz = 120000000,
		.spi_baudRate_hz = 2000000,
		.time_until_first_valid_SPCK = 0,
		.spi_mode = MODE_0
	};
	spi_masterInit(SpiMasterSettings);
	spi_deviceInit(Spi0);

Chip selects: 
The way the spi is configured the chip select will always remain low after a transfer until you either 
start transferring to a different chip select or send a buffer with the last transfer bit set.

The buffers that are given to the pdc registers work the following way:

[xxxxxxx(7-bit) + LASTXFER(1-bit)(1)+ xxxx(4-bit) + PCS (4-bit) + DATA (8 to 16-bit)]

The LASXFER bit tells the spi to deactivate the chipselect after the current data has been sent. 
The PCS field is the chip select which can be set to 0-3 depending on which peripheral you want to send to.
The data field will be from 8-16 bits depending on what you set the bitlength parameter to in the init.

The user can construct an array of these buffers and then pass them onto the pdc. Example of constructing buffers:

uint32_t tbuffer[3] = { 0x000(CS)(0..3) DATA1, 0x000(CS) DATA2, 0x01(LASTEXFER) 0 (CS) DATA3};

The receive buffers must be the same size as the transmit buffer:
uint32_t rbuffer[3] = {0};
	
There is a function included with the driver called spi_word which will create the words in the buffer for you:
uint32_t tbuffer[3] = { spi_word(false, CS = 2, data1) , spi_word(false, CS = 2, data2), spi_word(true, CS = 2, data3)};
	
The received data will be the 8-16 LSB bits in the receive buffer index corresponding to the transmit buffer index.


To use this driver you must create the binary sempahore and the mutex. These are declared extern for this purpose. This can be done before the schedueler starts. For example in main as follows:
spi_handlerIsDoneSempahore = xSemaphoreCreateBinary();
spi_mutex = xSemaphoreCreateMutex();

Both binary sempahores and mutexes in freeRTOS are created so that the first call to semaphoretake will pass. This is unwanted behaviour for us
since we want the spi_handlerIsDoneSempahore to be given from the SPI_Handler the first time. 

The solution is to call xSemaphoreTake(spi_handlerIsDoneSempahore,0); in some task before the first spi transmission.
*/

#endif /* SPI_H_ */