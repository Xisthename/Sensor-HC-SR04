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

	tc_init(TC0,2,0);				/* Enable timer block 1, channel 3, TCLK1 (MCK/2) and capturemode */
	tc_set_block_mode(TC0,2);
	tc_stop(TC0, 2);				/* Making sure the timer does not run  */
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


/*
 * Variables used in the following functions
 * --------------------
 * Function: hcsr04_get_distance_filtered
 * Function: hcsr04_get_distance_filtered_spikes 
 */
int const how_many_samples = 10;
int const how_many_legit_samples = 10 - 2;
int const min_value = 2;
int const max_value = 30;
int check_sample = 0;
int legit_sample_counter = 0;
int sum_values = 0;

/*
 * Function: hcsr04_get_distance_filtered
 * --------------------
 * Returns the average of 10 samples or -1 if more than 2 samples are outside the desired range (currently 2-50)
 */
int hcsr04_get_distance_filtered(void) {
	check_sample = 0;
	legit_sample_counter = 0;
	sum_values = 0;
	
	for (int i = 0; i < how_many_samples; i++) { // Loops through 10 samples
		while(!hcsr04_sample_ready()); // Waits for a new sample
		check_sample = hcsr04_get_distance(); // Stores the sample
		sum_values += check_sample; // Adds the sample's value to sum_values
		
		if (check_sample > min_value && check_sample < max_value) {
			legit_sample_counter++; // The sample is within within the desired range (currently 2-50)
		}
	}
	
	if (legit_sample_counter >= how_many_legit_samples) { // Checks if we got enough of legit samples
		return (sum_values / how_many_samples); // Returns the average of the 10 samples
	}
	return -1;
}


/*
 * Function: hcsr04_get_distance_filtered_spikes 
 * --------------------
 * Compares 10 samples from the hcsr04 sensor with one another which is used to filter spikes
 * Returns the average of the reliable data or -1 when the data is not reliable (3 out of 10 samples differ more than the filter_constant)
 */
int hcsr04_get_distance_filtered_spikes(void)
{
	int samples[how_many_samples];
	int reliable_data_array[how_many_samples];
	int count_index = 0;
	int filter_differ_constant = 4; // Differ constant
	
	check_sample = 0;
	legit_sample_counter = 0;
	sum_values = 0;

	for (int i = 0; i < how_many_samples; i++) {
		while(!hcsr04_sample_ready()); // Waits for a new sample
		check_sample = hcsr04_get_distance(); // Stores the sample

		if (check_sample > min_value && check_sample < max_value) {
			samples[i] = check_sample; // // The sample is within within the desired range (currently 2-50) and stored into an array
		}
	}

	/* Compares all samples with one another and samples that do NOT differ(4) more than 2 times compared to the other samples gets stored into reliable_data_array */
	for (int i = 0; i < how_many_samples; i++) {
		for (int j = 0; j < how_many_samples - 1; j++) {
			if (i != j + 1) { // Don't compare 
				/* Sample A compared to Sample B is within +- filter_differ_constant(4) 
				 * Where A is all samples except the last sample and
				 * Where B is all samples except the first sample
				 */
				if (samples[i] > (samples[j + 1] - filter_differ_constant) && samples[i] < (samples[j + 1] + filter_differ_constant)) {
					legit_sample_counter++;

					if (legit_sample_counter >= how_many_legit_samples) {
						reliable_data_array[count_index] = samples[i]; // Store a sample into reliable_data_array
						count_index++; // Keep track of the index to reliable_data_array
					}
				}
			}
		}
		legit_sample_counter = 0;
	}
	count_index = 0;

	for (int i = 0; i < how_many_samples; i++) {
		if (reliable_data_array[i] != 0) {
			sum_values += reliable_data_array[i];
			count_index++;
		}
	}

	if (count_index != 0) {
		return (sum_values / count_index);
	}
	return -1;
}
