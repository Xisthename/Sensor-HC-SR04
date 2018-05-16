/*
 * delay.c
 *
 * In order to avoid conflict with timer used for signal processing we use the other timer block here.
 * 
 * Author: Uek, Philip Ekholm
 */

#include "asf.h"
#include "delay.h"
#include "pin_mapper.h"

int delay_init(void) {
	pmc_enable_periph_clk(ID_TC1);	/* power on the peripheral clock for timers */
	tc_init(TC0, 1, 0);				/* Enable timer block 0, channel 1, TCLK1 och capturemode */
	tc_set_block_mode(TC0, 1);
	tc_stop(TC0, 1);					/* making sure the timer does not run  */
	return 0;						/* evertyhing is ok, typical response */
}

/* Beware that function is not thread safe, must only be called within one instance */
void delay_us(uint32_t us){
	tc_start(TC0, 1);
	while (tc_read_cv(TC0, 1) < us * 42);
	tc_stop(TC0, 1);
}

void delay_ms(uint32_t ms) {
	delay_us(1000 * ms);
}

void delay_s(uint32_t s) {
	delay_ms(1000 * s);
}

