/*
 * pio.c
 *
 * Created: 06.02.2015
 *  Author: Kristian Roaldsnes
 */ 

#include "pio.h"
#include "sam.h"
#include "pmc.h"
#include <stdbool.h>

static void pio_enablePullup			( Pio * pio, uint8_t pin);
static void pio_enablePulldown			( Pio * pio, uint8_t pin);
static void pio_disablePull				( Pio * pio, uint8_t pin);
static void pio_enableWriteProtection	( Pio * pio );
static void pio_disableWriteProtection	( Pio * pio );

#define WPKEY_ENABLE	0x50494F01
#define WPKEY_DISABLE	0x50494F00


static void (*pioaInterruptHooks[32])();
static void (*piobInterruptHooks[32])();
static void (*piocInterruptHooks[32])();



void pio_init()
{
	pmc_enable_periph_clk(PIOA_IRQn);
	pmc_enable_periph_clk(PIOB_IRQn);
	pmc_enable_periph_clk(PIOC_IRQn);

	
	NVIC_EnableIRQ(PIOA_IRQn);
	NVIC_EnableIRQ(PIOB_IRQn);
	NVIC_EnableIRQ(PIOC_IRQn);

}


void pio_enableOutput(Pio * pio, uint8_t pin ){
	
	pio_disableWriteProtection(pio);
	pio->PIO_OER = ( 1 << pin );
	pio_enableWriteProtection(pio);
}

void pio_disableOutput(Pio * pio, uint8_t pin ){
	
	pio_disableWriteProtection(pio);
	pio->PIO_ODR = ( 1 << pin );
	pio_enableWriteProtection(pio);
}

void pio_setOutput(Pio * pio, uint8_t pin, enum PinLevel pinlevel ){
		
	switch(pinlevel){
		case(PIN_LOW):
			pio->PIO_CODR = (1<<pin);
			break;
		case(PIN_HIGH):
			pio->PIO_SODR = (1<<pin);
			break;
	}
}



void pio_setMux(Pio * pio, uint8_t pin, enum Peripheral mux)
{
	pio_disableWriteProtection(pio);
	switch(mux){
		case PIO:
			pio->PIO_PER = (1<<pin);
			break; 
		case A:
			pio->PIO_PDR = (1<<pin);
			pio->PIO_ABCDSR[0] &= ~(1<<pin);
			pio->PIO_ABCDSR[1] &= ~(1<<pin);
			break;
		case B: 
			pio->PIO_PDR = (1<<pin); 
			pio->PIO_ABCDSR[0] |= (1<<pin);
			pio->PIO_ABCDSR[1] &= ~(1<<pin);
			break;
		case C:
			pio->PIO_PDR = (1<<pin);
			pio->PIO_ABCDSR[0] &= ~(1<<pin);
			pio->PIO_ABCDSR[1] |= (1<<pin);
			break;
		case D:
			pio->PIO_PDR = (1<<pin);
			pio->PIO_ABCDSR[0] |= (1<<pin);
			pio->PIO_ABCDSR[1] |= (1<<pin);
			break;
	}
	pio_enableWriteProtection(pio);
}


void pio_setPull ( Pio * pio, uint8_t pin, enum PullType pulltype ){
	
		switch(pulltype){
			case PULLUP:
				pio_enablePullup(pio,pin);
				break;
				
			case PULLDOWN:
				pio_enablePulldown(pio,pin);
				break;	
				
			case NOPULL:
				pio_disablePull(pio,pin);
				break;
				
			default:
				//do nothing
				break;
		}
};



bool pio_readPin(Pio * pio, uint8_t pin)
{
	if(pio->PIO_PDSR & (1<<pin))
		return true;
	else
		return false;
}

void pio_setFilter(Pio * pio, uint8_t pin, enum FilterType filter)
{
	pio_disableWriteProtection(pio);
	switch(filter){
		case NONE:
			pio->PIO_IFDR = (1<<pin);
			break;
		case GLITCH:
			pio->PIO_IFER = (1<<pin);
			pio->PIO_IFSCDR = (1<<pin);
			break;
		case DEBOUNCE:
			pio->PIO_IFER = (1<<pin);
			pio->PIO_IFSCER = (1<<pin);
			break;
	}
	pio_enableWriteProtection(pio);
}

void pio_setFallingEdgeInterrupt(Pio * pio, uint8_t pin)
{
	pio->PIO_AIMER = (1<<pin);
	pio->PIO_ESR = (1<<pin);
	pio->PIO_FELLSR = (1<<pin);
}

void pio_setRisingEdgeInterrupt(Pio * pio, uint8_t pin)
{
	pio->PIO_AIMER = (1<<pin);
	pio->PIO_ESR = (1<<pin);
	pio->PIO_REHLSR = (1<<pin);
}

void pio_setLowLevelInterrupt(Pio * pio, uint8_t pin)
{
	pio->PIO_AIMER = (1<<pin);
	pio->PIO_LSR = (1<<pin);
	pio->PIO_FELLSR = (1<<pin);
}

void pio_setHighLevelInterrupt(Pio * pio, uint8_t pin)
{
	pio->PIO_AIMER = (1<<pin);
	pio->PIO_LSR = (1<<pin);
	pio->PIO_REHLSR = (1<<pin);
}


void pio_enableInterrupt(Pio * pio, uint8_t pin, enum InterruptType interruptType, void (*interruptFunction)(void))
{
	pio_disableWriteProtection(pio);
	
	if(pio == PIOA){
		pioaInterruptHooks[pin] = interruptFunction;
	} else if(pio == PIOB){
		piobInterruptHooks[pin] = interruptFunction;
	} else if(pio == PIOC){
		piocInterruptHooks[pin] = interruptFunction;
	} 	
	switch(interruptType){
		case FALLING_EDGE:
			pio_setFallingEdgeInterrupt(pio, pin);
			break;
		case RISING_EDGE:
			pio_setRisingEdgeInterrupt(pio, pin);
			break;
		case LOW_LEVEL:
			pio_setLowLevelInterrupt(pio, pin);
			break;
		case HIGH_LEVEL:
			pio_setHighLevelInterrupt(pio, pin);
			break;
	}
	
	volatile uint32_t deleteInterrupts = pio->PIO_ISR;
	
	pio->PIO_IER =  ( 1 << pin );
	
	pio_enableWriteProtection(pio);
}

void pio_disableInterrupt( Pio * pio, uint8_t pin)
{
	pio_disableWriteProtection(pio);
	pio->PIO_IDR = ( 1 << pin );
	pio_enableWriteProtection(pio);
}

void pio_enablePullup(Pio * pio, uint8_t pin)
{
	pio_disableWriteProtection(pio);
	pio->PIO_PPDDR = (1<<pin); // disable pulldown
	pio->PIO_PUER = (1<<pin); // enable pullup
	pio_enableWriteProtection(pio);
}

void pio_enablePulldown(Pio * pio, uint8_t pin)
{
	pio_disableWriteProtection(pio);
	pio->PIO_PUDR = (1<<pin); // disable pullup
	pio->PIO_PPDER = (1<<pin); // enable pulldown
	pio_enableWriteProtection(pio);
}

void pio_disablePull(Pio * pio, uint8_t pin)
{
	pio_disableWriteProtection(pio);
	pio->PIO_PUDR = (1<<pin); // disable pullup
	pio->PIO_PPDDR = (1<<pin); // disable pulldown
	pio_enableWriteProtection(pio);
}


void pio_enableWriteProtection( Pio * pio ){
	pio->PIO_WPMR = WPKEY_ENABLE;
}

void pio_disableWriteProtection( Pio * pio ){
	pio->PIO_WPMR = WPKEY_DISABLE;
}



void PIOA_Handler()
{
	uint32_t interruptMask = PIOA->PIO_ISR & PIOA->PIO_IMR;
	
	for(int i = 0; i < 32; i++){
		if(interruptMask & (1<<i)){
			(*pioaInterruptHooks[i])();
		}
	}
}

void PIOB_Handler()
{
	uint32_t interruptMask = PIOB->PIO_ISR & PIOB->PIO_IMR;
	
	for(int i = 0; i < 32; i++){
		if(interruptMask & (1<<i)){
			(*piobInterruptHooks[i])();
		}
	}
}

void PIOC_Handler()
{
	uint32_t interruptMask = PIOC->PIO_ISR & PIOC->PIO_IMR;
	
	for(int i = 0; i < 32; i++){
		if(interruptMask & (1<<i)){
			(*piocInterruptHooks[i])();
		}
	}
}
