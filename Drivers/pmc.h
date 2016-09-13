/**
 * \file
 *
 * \brief Power Management Controller (PMC) driver for SAM.
 *
 * Copyright (c) 2011 - 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef PMC_H_INCLUDED
#define PMC_H_INCLUDED

#include "sam.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond


/** Bit mask for peripheral clocks (PCER0) */
#define PMC_MASK_STATUS0        (0xFFFFFFFC)

/** Bit mask for peripheral clocks (PCER1) */
#define PMC_MASK_STATUS1        (0xFFFFFFFF)

/** Loop counter timeout value */
#define PMC_TIMEOUT             (2048)

/** Key to unlock CKGR_MOR register */
#ifndef CKGR_MOR_KEY_PASSWD
#define CKGR_MOR_KEY_PASSWD    CKGR_MOR_KEY(0x37U)
#endif

/** Key used to write SUPC registers */
#ifndef SUPC_CR_KEY_PASSWD
#define SUPC_CR_KEY_PASSWD    SUPC_CR_KEY(0xA5U)
#endif

#ifndef SUPC_MR_KEY_PASSWD
#define SUPC_MR_KEY_PASSWD    SUPC_MR_KEY(0xA5U)
#endif
// 
// /** Mask to access fast startup input */
// #define PMC_FAST_STARTUP_Msk    (0x7FFFFu)

/** PMC_WPMR Write Protect KEY, unlock it */
#ifndef PMC_WPMR_WPKEY_PASSWD
#define PMC_WPMR_WPKEY_PASSWD    PMC_WPMR_WPKEY((uint32_t) 0x504D43)
#endif

// /** Using external oscillator */
// #define PMC_OSC_XTAL            0
// 
// /** Oscillator in bypass mode */
// #define PMC_OSC_BYPASS          1
// 
// #define PMC_PCK_0               0 /* PCK0 ID */
// #define PMC_PCK_1               1 /* PCK1 ID */
// #define PMC_PCK_2               2 /* PCK2 ID */
// 
// /** Flash state in Wait Mode */
// #define PMC_WAIT_MODE_FLASH_STANDBY         PMC_FSMR_FLPM_FLASH_STANDBY
// #define PMC_WAIT_MODE_FLASH_DEEP_POWERDOWN  PMC_FSMR_FLPM_FLASH_DEEP_POWERDOWN
// #define PMC_WAIT_MODE_FLASH_IDLE            PMC_FSMR_FLPM_FLASH_IDLE

// 
// /** Convert startup time from us to MOSCXTST */
// #define pmc_us_to_moscxtst(startup_us, slowck_freq)      \
// 	((startup_us * slowck_freq / 8 / 1000000) < 0x100 ?  \
// 		(startup_us * slowck_freq / 8 / 1000000) : 0xFF)


typedef enum {INTERNAL_4MHZ = CKGR_MOR_MOSCRCF_4_MHz, 
	INTERNAL_8MHZ = CKGR_MOR_MOSCRCF_8_MHz, 
	INTERNAL_12MHZ = CKGR_MOR_MOSCRCF_12_MHz, 
	EXTERNAL
	} MainClockFrequency;

typedef enum {CLK_1 = PMC_MCKR_PRES_CLK_1,
	CLK_2 = PMC_MCKR_PRES_CLK_2,
	CLK_4 = PMC_MCKR_PRES_CLK_4,
	CLK_8 = PMC_MCKR_PRES_CLK_8,
	CLK_16 = PMC_MCKR_PRES_CLK_16,
	CLK_32 = PMC_MCKR_PRES_CLK_32,
	CLK_64 = PMC_MCKR_PRES_CLK_64,
	CLK_3 = PMC_MCKR_PRES_CLK_3
	} ProcessorClockPrescaler;

typedef enum {SLOW_CLOCK = PMC_MCKR_CSS_SLOW_CLK,
	MAIN_CLOCK = PMC_MCKR_CSS_MAIN_CLK,
	PLLA_CLOCK = PMC_MCKR_CSS_PLLA_CLK
	} MasterClockSource;

struct PmcInit {
	MainClockFrequency freq;
	MasterClockSource css;
	ProcessorClockPrescaler pres;
	uint8_t divide;
	uint8_t multiply;
};

void pmc_enable_usb_clock(int divide);
void pmc_disable_usb_clock(void);
uint32_t pmc_enable_periph_clk(uint32_t irqnNumber);
uint32_t pmc_disable_periph_clk(uint32_t irqnNumber);
uint32_t pmc_init(struct PmcInit pmc_init_struct);

#endif /* PMC_H_INCLUDED */
