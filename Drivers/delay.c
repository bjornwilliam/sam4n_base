/*
 * delay.c
 *
 * Created: 30.09.2014 01:27:39
 *  Author: Kjetil
 */ 

#include "delay.h"
#include "pmc.h"

#define CPU_CLOCK_FREQ_HZ 100000000

// sjekk register eventuelt bruke cycles i assembly
void delay_clk( __attribute__((unused)) volatile uint32_t cycles) // is this safe or can cycles be optimized away?? worked so far....
{
	// R0 and R1 can be used freely inside the function, referance ARM calling convetion
	asm volatile("MOV R1, #1"); // 3 clock cycles per loop
	asm volatile("UDIV R0, R1"); // unsigned division
	asm volatile("loop: SUBS R0, R0, #1");
	asm volatile("BNE loop");
	//asm("");
}

void delay_ms(uint32_t ms)
{
	delay_clk(ms*(CPU_CLOCK_FREQ_HZ/1000));
}

void delay_us(uint32_t us)
{
	delay_clk(us*(CPU_CLOCK_FREQ_HZ/1000000));
}
