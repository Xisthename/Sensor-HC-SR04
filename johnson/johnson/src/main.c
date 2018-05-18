/*
 * P2 Johnson Due
 *
 * Created: 2015-06-17 14:32:55
 *  Author: Philip Ekholm, 
 */ 

#include <asf.h>

#include "motor_task.h"
#include "main_task.h"

#include "drivers/console_driver.h"
#include "drivers/encoder.h"
#include "pin_mapper.h"
#include "MotorControll.h"
#include "config/PWM_Configuration.h"
#include "drivers/TWI.h"
#include "drivers/hcsr04.h"
#include "drivers/delay_driver.h"

int main (void)
{
	sysclk_init();
	board_init();
	delay_driver_init();
	ioport_init();
	console_init();
	encoder_init();
	PWM_init();
	Twi_master_init(TWI1);
	hcsr04_init();
	
	ioport_enable_pin(pin_mapper(TASK_DEBUG_MOTOR_PIN));
	ioport_enable_pin(pin_mapper(TASK_DEBUG_MAIN_PIN));
	ioport_enable_pin(pin_mapper(30));
	ioport_enable_pin(pin_mapper(SWITCH_CURIE_NOETHER_PIN)); /* Switch for switching platforms */

	ioport_set_pin_dir(pin_mapper(TASK_DEBUG_MOTOR_PIN), IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(pin_mapper(TASK_DEBUG_MAIN_PIN), IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(pin_mapper(SWITCH_CURIE_NOETHER_PIN), IOPORT_DIR_INPUT);
	/* Enable pullup for switch as well */
	PIOC->PIO_PUER |= PIO_PC25;

	/* Create queue-handle for the motor task */
	motor_task_instruction_handle = xQueueCreate(1, sizeof(struct motor_task_instruction));
 
	/* Create our tasks for the program */
	xTaskCreate(motor_task, (const signed char * const) "motor_task", TASK_MOTOR_STACK_SIZE, NULL, TASK_MOTOR_PRIORITY, NULL);
	xTaskCreate(main_task, (const signed char * const) "main_task", TASK_MAIN_STACK_SIZE, NULL, TASK_MAIN_PRIORITY, NULL);

	vTaskStartScheduler();

	return 0;
}
