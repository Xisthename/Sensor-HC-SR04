#include <asf.h>

#include "pin_mapper.h"
#include "drivers/delay.h"
#include "drivers/console_driver.h"
#include "drivers/hcsr04.h"

int main (void) {
	sysclk_init();
	board_init();

	ioport_init();
	delay_init();
	console_init();
	
	ioport_enable_pin(pin_mapper(13));
	ioport_set_pin_dir(pin_mapper(13), IOPORT_DIR_OUTPUT);
	
	hcsr04_init();

	while(1) {
		printf("Sample: %d\n", hcsr04_get_distance());
	}
}
