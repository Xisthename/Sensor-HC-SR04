/*
 * pins.c
 *
 * Created: 2018-04-16 21:09:56
 */ 
#include <asf.h>
#include <sam3x8e.h>


#include "encoder.h"
#include "MotorControll.h"

static volatile int counterA = 0;
static volatile int counterB = 0;


int get_counterA(void) {
	return counterA;
}

int get_counterB(void) {
	return counterB;
}

void resetCounterA() {
	counterA = 0;
}

void resetCounterB() {
	counterB = 0;
}

void pio_interruptA(void) {
	// Save all triggered interrupts
	uint32_t status1 = PIOA->PIO_ISR;
	counterA++;
}

void pio_interruptB(void) {
	// Save all triggered interrupts
	uint32_t status2 = PIOD->PIO_ISR;
	counterB++;
}

void encoder_init(void) {
	// Enable Clock for PIOB - needed for sampling falling edge
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOD);
	
	pio_set_input(PIOA, PIO_PA15, PIO_PULLUP); 
	pio_set_input(PIOD, PIO_PD0, PIO_PULLUP);
	
	// Enable Glitch/Debouncing filter
	PIOA->PIO_IFER = PIO_PA15;
	PIOD->PIO_IFER = PIO_PD0;
	
	// Select Debouncing filter
	PIOA->PIO_DIFSR = PIO_PA15;
	PIOD->PIO_DIFSR = PIO_PD0;
	
// 	if(getVenster() < 1500){
// 		pio_handler_set(PIOA, ID_PIOA, PIO_PA15, PIO_IT_FALL_EDGE, pio_interruptA);
// 	}
// 	else{
		pio_handler_set(PIOA, ID_PIOA, PIO_PA15, PIO_IT_RISE_EDGE, pio_interruptA);
//	}
	pio_enable_interrupt(PIOA, PIO_PA15);


// 	if(getHoger() < 1500){
// 		pio_handler_set(PIOD, ID_PIOD, PIO_PD0, PIO_IT_FALL_EDGE, pio_interruptB);
// 	}
// 	else{
		pio_handler_set(PIOD, ID_PIOD, PIO_PD0, PIO_IT_RISE_EDGE, pio_interruptB);
//	}
	
	pio_enable_interrupt(PIOD, PIO_PD0);
	
	// Enable Interrupt Handling in NVIC
	NVIC_EnableIRQ(PIOA_IRQn);
	NVIC_EnableIRQ(PIOD_IRQn);
}
