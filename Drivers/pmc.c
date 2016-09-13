#include "pmc.h"
#include <sam.h>

#define MAX_PERIPH_ID    47


#ifdef __cplusplus
extern "C" {
#endif


void pmc_enable_writeprotect(void)
{
	PMC->PMC_WPMR = PMC_WPMR_WPKEY_PASSWD | PMC_WPMR_WPEN;
}

void pmc_disable_writeprotect(void)
{
	PMC->PMC_WPMR = PMC_WPMR_WPKEY_PASSWD;
}

void pmc_mck_set_prescaler(uint32_t pres)
{
	PMC->PMC_MCKR = (PMC->PMC_MCKR & (~PMC_MCKR_PRES_Msk)) | pres;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));
}


void pmc_mck_set_source(uint32_t css)
{
	PMC->PMC_MCKR = (PMC->PMC_MCKR & (~PMC_MCKR_CSS_Msk)) | css;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));
}


void pmc_switch_mainck_to_fastrc(void)
{
	PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCSEL) | CKGR_MOR_KEY_PASSWD;
}

void pmc_enable_fastrc(void)
{
	PMC->CKGR_MOR |= (CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCEN);
	while (!(PMC->PMC_SR & PMC_SR_MOSCRCS)); 	/* Wait the Fast RC to stabilize */
}

void pmc_set_fastrc_frequency(uint32_t moscrcf)
{
	PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCRCF_Msk) | CKGR_MOR_KEY_PASSWD | moscrcf;
	while (!(PMC->PMC_SR & PMC_SR_MOSCRCS)); /* Wait the Fast RC to stabilize */
}

void pmc_disable_fastrc(void)
{
	PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCRCEN & ~CKGR_MOR_MOSCRCF_Msk) | CKGR_MOR_KEY_PASSWD;
}

uint32_t pmc_osc_is_ready_fastrc(void)
{
	return (PMC->PMC_SR & PMC_SR_MOSCRCS);
}

uint32_t pmc_xtal_ready(void)
{
	return (PMC->PMC_SR & PMC_SR_MOSCXTS);
}

void pmc_enable_main_xtal(uint32_t xtalStartupTime)
{
	uint32_t mor = PMC->CKGR_MOR;
	mor &= ~(CKGR_MOR_MOSCXTBY|CKGR_MOR_MOSCXTEN);
	mor |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTEN |
	CKGR_MOR_MOSCXTST(xtalStartupTime);
	PMC->CKGR_MOR = mor;
	/* Wait the main Xtal to stabilize */
	while (!pmc_xtal_ready());
}

void pmc_switch_mainck_to_xtal(void)
{
	PMC->CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL;
	while(!pmc_xtal_ready());
}

uint32_t pmc_mainck_ready(void)
{
	return PMC->PMC_SR & PMC_SR_MOSCSELS;
}

void pmc_disable_pllack(void)
{
	PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | CKGR_PLLAR_MULA(0);
}

uint32_t pmc_plla_is_locked(void)
{
	return (PMC->PMC_SR & PMC_SR_LOCKA);
}

void pmc_enable_pllack(uint32_t mula, uint32_t pllacount, uint32_t diva)
{
	/* first disable the PLL to unlock the lock */
	pmc_disable_pllack();

	PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | CKGR_PLLAR_DIVA(diva) | CKGR_PLLAR_PLLACOUNT(pllacount) | CKGR_PLLAR_MULA(mula-1);
	if(diva == 0 || mula == 0)
	return;

	while (!pmc_plla_is_locked());
}


void pmc_enable_interrupt(uint32_t ul_sources)
{
	PMC->PMC_IER = ul_sources;
}

void pmc_disable_interrupt(uint32_t ul_sources)
{
	PMC->PMC_IDR = ul_sources;
}

uint32_t pmc_get_interrupt_mask(void)
{
	return PMC->PMC_IMR;
}

void pmc_enable_clock_failure_detector(void)
{
	PMC->CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_CFDEN;
}

void pmc_disable_clock_failure_detector(void)
{
	PMC->CKGR_MOR = (PMC->CKGR_MOR & (~CKGR_MOR_CFDEN)) | CKGR_MOR_KEY_PASSWD;
}

void pmc_select_main_clock(MainClockFrequency freq)
{
	switch(freq){
		case INTERNAL_4MHZ:
		case INTERNAL_8MHZ:
		case INTERNAL_12MHZ:
			
		pmc_enable_fastrc();
		pmc_set_fastrc_frequency(freq);
		pmc_switch_mainck_to_fastrc();
			
		break;
			
		case EXTERNAL:
		pmc_enable_main_xtal(0xff);
		pmc_switch_mainck_to_xtal();
		pmc_disable_fastrc();
			
		break;
	}
}

void pmc_select_master_clock(MasterClockSource css, ProcessorClockPrescaler pres)
{
	if(css == PLLA_CLOCK){
		pmc_mck_set_prescaler(pres);
		pmc_mck_set_source(css);
		} else {
		pmc_mck_set_source(css);
		pmc_mck_set_prescaler(pres);
	}
}

uint32_t pmc_disable_periph_clk(uint32_t irqnNumber)
{
		
	if (irqnNumber > MAX_PERIPH_ID)
	return 1;
	pmc_disable_writeprotect();
	if (irqnNumber < 32) {
		if ((PMC->PMC_PCSR0 & (1u << irqnNumber)) == (1u << irqnNumber))
		PMC->PMC_PCDR0 = 1 << irqnNumber;
		} else {
		irqnNumber -= 32;
		if ((PMC->PMC_PCSR0 & (1u << irqnNumber)) == (1u << irqnNumber))
		PMC->PMC_PCDR0 = 1 << irqnNumber;
	}
	pmc_enable_writeprotect();
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////


uint32_t pmc_enable_periph_clk(uint32_t irqnNumber)
{
	if (irqnNumber > MAX_PERIPH_ID)
	return 1;

	pmc_disable_writeprotect();
	if (irqnNumber < 32) {
		if ((PMC->PMC_PCSR0 & (1 << irqnNumber)) != (1 << irqnNumber))
		PMC->PMC_PCER0 = (1 << irqnNumber);
		} else {
		irqnNumber -= 32;
		if ((PMC->PMC_PCSR0 & (1 << irqnNumber)) != (1 << irqnNumber))
		PMC->PMC_PCER0 = (1 << irqnNumber);
	}
	pmc_enable_writeprotect();

	return 0;
}

uint32_t pmc_init(struct PmcInit pmc_init_struct)
{
	pmc_disable_writeprotect();
	pmc_select_main_clock(pmc_init_struct.freq);
	pmc_enable_pllack(pmc_init_struct.multiply, 0x3f, pmc_init_struct.divide);
	pmc_select_master_clock(pmc_init_struct.css, pmc_init_struct.pres);
	pmc_enable_writeprotect();

	return 0;
}



#ifdef __cplusplus
}
#endif

