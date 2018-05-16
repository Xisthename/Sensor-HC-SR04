/*
 * hcsr04.c
 *
 * Created: 5/8/2018 8:01:29 AM
 * Author: Philip Ekholm
 * Updated by Daniel date
 * Updated by Emil date
 *
 *
 * NOTE by Daniel: Pin 6 and 7 are blocked by the regulator part
 *
 */ 
#include <asf.h>
#include <sam3x8e.h>

#include "hcsr04.h"
#include "pin_mapper.h"
#include "delay.h"

#define TRIG_PIN_DUE 3
#define ECHO_PIN_DUE 4

#define TRIG_PIN_Middle PIO_PC28_IDX // pin 3
#define ECHO_PIN_Middle PIO_PC26 // pin 4

static volatile int last_sample = 0;

int hcsr04_get_distance(void) {
	return last_sample;
}

void TC0_Handler(void)
{
	ioport_set_pin_level(pin_mapper(13), 1);
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0, 0);		//The compare bit is cleared by reading the register, manual p. 915
	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	/* Send TRIG pulse for starting sample */
	ioport_set_pin_level(pin_mapper(TRIG_PIN_DUE), 1); // pin_mapper(TRIG_PIN_DUE)
	delay_us(9); /* Pulse specified to be 10 us wide and */
	ioport_set_pin_level(pin_mapper(TRIG_PIN_DUE), 0); // pin_mapper(TRIG_PIN_DUE)
	
	ioport_set_pin_level(pin_mapper(13), 0);
}

void edge_detection(void) {
	uint32_t pulse_width_us;

	/* Was it rising edge? start the timer */
	if (pio_get(PIOC, PIO_TYPE_PIO_INPUT, ECHO_PIN_Middle)) {
		/* Start TC to measure pulse width */
		tc_start(TC0, 2);
	}
	/* Falling edge, stop timer */
	else {
		pulse_width_us = tc_read_cv(TC0, 2) / 42;
		/* Pulse width proportionate against pulse width / 58, see HC-SR04 datasheet */
		last_sample = pulse_width_us / 58;
		tc_stop(TC0, 2);
	}
}

/*
* Initialize the timer
*/

static void timer_sample_init(void)
{
	/* Configure PMC */
	pmc_enable_periph_clk(ID_TC0);

	tc_init(TC0, 0, 0 | TC_CMR_CPCTRG);			//Timer_clock_1 - MCK/2 - 42 MHz
	tc_write_rc(TC0, 0, 4200000);				//4 200 000 corresponds to fs = 10 Hz

	/* Configure and enable interrupt on RC compare */
	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);
	
	tc_start(TC0, 0);
}

static void timer_measure_init(void) {
	/* power on the peripheral clock for timers */
	pmc_enable_periph_clk(ID_TC2);

	tc_init(TC0, 2, 0);				/* Enable timer block 3, channel 0, TCLK1 (MCK/2) and capturemode */
	tc_set_block_mode(TC0, 2);
	tc_stop(TC0, 2);				/* making sure the timer does not run  */
}

/*
* Enable interrupt to be triggered on rising/falling edge on input pin
*/
static void echo_init_pin_interrupts() {
	/* Enable Clock for PIOB - needed for sampling falling edge */
	pmc_enable_periph_clk(ID_PIOC);		

	/* Set specific pin to use as input */
	pio_set_input(PIOC, ECHO_PIN_Middle, PIO_PULLUP);

	/* Unfortunately we can only have one interrupt handler per pin */
	pio_handler_set(PIOC, ID_PIOC, ECHO_PIN_Middle, PIO_IT_EDGE, edge_detection);

	/* Enable Interrupt Handling in NVIC and in PIO */
	pio_enable_interrupt(PIOC, ECHO_PIN_Middle);
	NVIC_EnableIRQ(PIOC_IRQn);
}


void set_pins() {
	ioport_enable_pin(TRIG_PIN_Middle);
	ioport_set_pin_dir(TRIG_PIN_Middle, IOPORT_DIR_OUTPUT);
	
	ioport_enable_pin(PIO_PC26_IDX);
	ioport_set_pin_dir(PIO_PC26_IDX, IOPORT_DIR_INPUT);
}


void hcsr04_init(void) {
	/* Set pins to inputs/outputs */
	set_pins();
	/* Enable timer interrupt for constant sampling at 10 Hz */
	timer_sample_init();
	/* Enable another timer to measure pulse width using capture mode */
	timer_measure_init();
	/* Enable the interrupts for measuring pulse width on ECHO */
	echo_init_pin_interrupts();
}