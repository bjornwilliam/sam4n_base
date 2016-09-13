/*
 * pio.h
 *
 * Created: 21.10.2014 22:55:02
 *  Author: Kjetil Kjeka
 */ 


#ifndef PIO_H_
#define PIO_H_

#include <sam.h>

#include <stdbool.h>

enum Peripheral		{ PIO,	  A, B, C, D		};
enum FilterType		{ NONE,	  DEBOUNCE, GLITCH	};
enum PinLevel		{ PIN_LOW,	  PIN_HIGH				};
enum PullType		{ PULLUP, PULLDOWN, NOPULL	};
enum InterruptType	{FALLING_EDGE, RISING_EDGE, LOW_LEVEL, HIGH_LEVEL};




void pio_init			();
void pio_enableOutput	( Pio * pio, uint8_t pin );
void pio_disableOutput	( Pio * pio, uint8_t pin );
void pio_setOutput		( Pio * pio, uint8_t pin, enum PinLevel setState);
void pio_setMux			( Pio * pio, uint8_t pin, enum Peripheral mux	);
void pio_setPull		( Pio * pio, uint8_t pin, enum PullType pull	);
bool pio_readPin		( Pio * pio, uint8_t pin );
void pio_setFilter		( Pio * pio, uint8_t pin, enum FilterType filter);
void pio_enableInterrupt(Pio * pio, uint8_t pin, enum InterruptType interruptType, void (*interruptFunction)(void));
void pio_disableInterrupt( Pio * pio, uint8_t pin);


#endif