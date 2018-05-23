/*
 * hcsr04.c
 *
 * Created: 5/8/2018 8:01:29 AM
 *  Author: Philip Ekholm
 */ 

#include <asf.h>
#include <sam3x8e.h>

#include "hcsr04.h"
#include "pin_mapper.h"
#include "delay.h"

#define TRIG_PIN_DUE 9
#define ECHO_PIN_DUE 12

static volatile int last_sample = 0;
static volatile int new_sample = 0;

int hcsr04_sample_ready(void) {
	return new_sample;
}

int hcsr04_get_distance(void) {
	new_sample = 0;
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
	ioport_set_pin_level(pin_mapper(TRIG_PIN_DUE), 1);
	delay_us(9); /* Pulse specified to be 10 us wide */
	ioport_set_pin_level(pin_mapper(TRIG_PIN_DUE), 0);
	
	ioport_set_pin_level(pin_mapper(13), 0);
	new_sample = 1;
}

void edge_detection(void) {
	uint32_t pulse_width_us;

	/* Was it rising edge? start the timer */
	if (pio_get(PIOD, PIO_TYPE_PIO_INPUT, PIO_PD8)) {
		/* Start TC to measure pulse width */
		tc_start(TC0,2);
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

	tc_init(TC0,2,0);				/* Enable timer block 1, channel 3, TCLK1 (MCK/2) och capturemode */
	tc_set_block_mode(TC0,2);
	tc_stop(TC0, 2);				/* making sure the timer does not run  */
}

/*
* Enable interrupt to be triggered on rising/falling edge on input pin
*/
static void echo_init_pin_interrupts(void) {
	/* Enable Clock for PIOB - needed for sampling falling edge */
	pmc_enable_periph_clk(ID_PIOD);

	/* Set specific pin to use as input */
	pio_set_input(PIOD, PIO_PD8, PIO_PULLUP);

	/* Unfortunately we can only have one interrupt handler per pin */
	pio_handler_set(PIOD, ID_PIOD, PIO_PD8, PIO_IT_EDGE, edge_detection);

	/* Enable Interrupt Handling in NVIC and in PIO */
	pio_enable_interrupt(PIOD, PIO_PD8);
	NVIC_EnableIRQ(PIOD_IRQn);
}


void hcsr04_init(void) {
	/* Enable timer interrupt for constant sampling at 10 Hz */
	timer_sample_init();
	/* Enable another timer to measure pulse width using capture mode */
	timer_measure_init();
	/* Enable the interrupts for measuring pulse width on ECHO */
	echo_init_pin_interrupts();
}

/* Observe function will "hang" while waiting for new sample */
int hcsr04_get_distance_filtered(void) {
	int max_value = 50;
	int min_value = 2;
	int check_sample = 0;
	int legit_sample_counter = 0;
	int sum_values = 0;
	
	for (int i = 0; i < 10; i++) {
		while(!hcsr04_sample_ready());
		check_sample = hcsr04_get_distance();
		sum_values += check_sample;
		
		if (check_sample > min_value && check_sample < max_value) {
			legit_sample_counter++;
		}
	}
	
	if (legit_sample_counter > 7) {
		return sum_values / 10;
	} 
	else {
		return -1;
	}
}

int hcsr04_get_distance_filtered_new(void)
{
	int max_value = 50;
	int min_value = 2;
	int check_sample = 0;
	int legit_sample_counter = 0;
	int sum_values = 0;
	int array[10];
	int reliable_data_array[10];
	int count = 0;
	int average_filter = 2;

	for (int i = 0; i < 10; i++)
	{
		while(!hcsr04_sample_ready());
		check_sample = hcsr04_get_distance();

		if (check_sample > min_value && check_sample < max_value)
		{
			array[i] = check_sample;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (i != j + 1)
			{
				if (array[i] > (array[j + 1] - average_filter) && array[i] < (array[j + 1] + average_filter))
				{
					legit_sample_counter++;

					if (legit_sample_counter > 6)
					{
						reliable_data_array[count] = array[i];
						count++;
					}
				}
			}
		}
		legit_sample_counter = 0;
	}
	count = 0;

	for (int i = 0; i < 10; i++)
	{
		if (reliable_data_array[i] != 0)
		{
			sum_values += reliable_data_array[i];
			count++;
		}
	}

	if (count != 0)
	{
		return (sum_values / count);
	}
	return -1;
}
