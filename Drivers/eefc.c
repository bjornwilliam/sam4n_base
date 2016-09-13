#define MASTER_CLOCK_FREQ 100000000

#include "sam.h"

void init_flash(void)
{	
	/* Set FWS for embedded Flash access according to operating frequency */
	if ( MASTER_CLOCK_FREQ < CHIP_FREQ_FWS_0 ) {
		EFC->EEFC_FMR = EEFC_FMR_FWS(0)|EEFC_FMR_CLOE;
		} else {
		if (MASTER_CLOCK_FREQ < CHIP_FREQ_FWS_1) {
			EFC->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
			} else {
			if (MASTER_CLOCK_FREQ < CHIP_FREQ_FWS_2) {
				EFC->EEFC_FMR = EEFC_FMR_FWS(2)|EEFC_FMR_CLOE;
				} else {
				if ( MASTER_CLOCK_FREQ < CHIP_FREQ_FWS_3 ) {
					EFC->EEFC_FMR = EEFC_FMR_FWS(3)|EEFC_FMR_CLOE;
					} else {
					if ( MASTER_CLOCK_FREQ < CHIP_FREQ_FWS_4 ) {
						EFC->EEFC_FMR = EEFC_FMR_FWS(4)|EEFC_FMR_CLOE;
						} else {
						EFC->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
					}
				}
			}
		}
	}
}