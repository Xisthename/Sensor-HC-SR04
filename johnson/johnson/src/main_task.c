/*
 * nav_task.c
 *
 * Created: 4/23/2018 11:35:40 AM
 * Author: Philip Ekholm
 * Updated: Ali Hassan
 * Updated: Daniel -> Sensor: Movement and rotation logic (2018-05-18) // Not done
 * Updated Emil -> Sensor: Filtering data samples (2018-05-18) // Done
 */
#include <asf.h>

#include "main_task.h"
#include "motor_task.h"
#include "pin_mapper.h"
#include "drivers/encoder.h"
#include "math_functions.h"
#include "positions.h"
#include "drivers/hcsr04.h"

#define MAIN_TASK_PERIODICITY 10

static struct point current_pos;
static struct point earlier_pos;
static struct point box;
static struct point object;

struct point mock_positions[4] = {
	{ 50, 100 },
	{ 120, 50 },
	{ 140, 150 },
	{ 280, 260 },
};

void main_task(void *pvParameters) {
	int16_t distance, minimum_distance_to_object; /* In cm */
	double alpha, beta, correction_angle, minimum_angle = 1; /* In degrees */
	uint8_t mock_pos_index = 0;
	
	/* Read first time package and set static coordinates */
	update_positions();
	current_pos = get_pos();
	box = get_box();
	
	if (ioport_get_pin_level(pin_mapper(SWITCH_CURIE_NOETHER_PIN))) {
		/* Marie Curie */
		object = get_cube();
		minimum_distance_to_object = 25; // The distance in front of the platform to the ball for picking it up
		printf("Curie");
	}
	else{
		/* Emmy Noether */
		object = get_ball();
		minimum_distance_to_object = 20; // The distance in front of the platform to the steel ball for picking it up
		printf("Noether");
	}
		
	update_positions();
	current_pos = get_pos();

	/* Drive only half the distance */
	distance = get_euclid_distance(object.x, object.y, current_pos.x, current_pos.y) / 2;
	alpha = math_get_angle_deg(math_atan2(object.x, object.y, current_pos.x, current_pos.y));
		
	struct motor_task_instruction inst = {
		.distance = distance,
		.angle = alpha
	};
		
	xQueueSend(motor_task_instruction_handle, &inst, 10);
	earlier_pos = current_pos;
	
	printf("First run, angle: %d, d: %d", (int16_t)alpha, distance);

	while(distance > minimum_distance_to_object) {
		/* Wait for motor task to complete */
		while(xQueuePeek(motor_task_instruction_handle, &inst, 2));

		//I2C_master_read(TWI1, &packet_pos);
		//PSEUDO: Get robot's new position and update current_pos
		current_pos = mock_positions[mock_pos_index];
		printf("Current pos: (%d, %d)\n", current_pos.x, current_pos.y);
		printf("Earlier pos: (%d, %d)\n", earlier_pos.x, earlier_pos.y);
		
		distance = get_euclid_distance(object.x, object.y, current_pos.x, current_pos.y) / 2;
		/* Calculate the actual angle that was driven */
		beta = math_get_angle_deg(math_atan2(current_pos.x, current_pos.y, earlier_pos.x, earlier_pos.y));
		alpha = math_get_angle_deg(math_atan2(object.x, object.y, current_pos.x, current_pos.y));
		correction_angle = beta - alpha;
		
		printf("correction: %d, d: %d\n", (int16_t)correction_angle, distance);
		printf("Object: (%d, %d)\n", object.x, object.y);

		if (abs(correction_angle) < minimum_angle){
			correction_angle = 0;
		}
			
		inst.distance = distance;
		inst.angle = correction_angle;

		if (xQueueSend(motor_task_instruction_handle, &inst, 5)) {
			/* Instruction successfully sent to motor task */
			earlier_pos = current_pos;
			mock_pos_index++;
		}
	}
	
	
	/* Sensor begin */ 
	/* 
	 * TODO Måste ta hänsyn till vad för upplocknings radie de två olika påbyggnaderna har gentemot avståndet man mäter objektet på
	 * Vinkeln från sensorn ska vara teoretiskt sett 15 grader utåt från båda hållen
	 * Stålkulan: 
	 * Mätvärden för 15 cm resulterar i en diameter på 12 cm och upplockningen för stålkulan är 11 cm
	 * Mätvärden för 18.6 cm resulterar i att objektet inte går att upptäcka om det inte är precis framför sensorn med 1-2 grader och därmed är det inget problem
	 *
	 *
	 * Träklosse: Mätvärden indikerar att ett avstånd på 19 cm resulterar i en diameter på 16 cm och påbyggnaden har fortfarande inte lämnat besked om deras diameter... 
	 *
	 *
	 * Hur man löser detta finns det två olika alternativ:
	 * Lösning 1: Gör som i tillståndsmaskinen med att bestämma ett säkert intervall (5-12 cm) och man kör fram till 10 cm om man får stabila värden på tex 30 cm
	 * Lösning 2: Man drar ner intervallet innan själva "filteringen" äger rum vilket gör att roboten blir "korkad" då den kan få värden som är över 30 cm 
	 * och sen måste den ändå börja med att rotera för att sedan köra en liten bit framåt vilket uppreppas tills den hittar objektet till slut
	 *
	 * Slutsats: 
	 * Implementerar en enkel lösning först och ser om detta teoretiska problem verkligen uppstår även i praktiken...
	 */
	
	int distance_to_move; // Distance the robot has to move for picking up an object
	int const step_forward = 5.6; // How much the robot will move forward in attempt to find an object
	int const distance_from_plattform = 6; // The sensor is located 6 cm in front of the robot
	
	float rotate = 0; // How much has the robot rotated in degree
	float const rotation_deg = 22.8; // The motor task can only rotate 3.8 degree as minimum
	int moved_counter = 0; // How many times the robot will move forward when it doesn't find an object
	int const max_moved_times = 5; // Max times the robot will move forward in attempt to find an object
	
	int check_samples(void) { // filtering (doesn't have to be a function)
		int max_value = 25;
		int min_value = 5;
		int check_sample = 0;
		int legit_sample_counter = 0;
		int sum_values = 0;
		
		for (int i = 0; i < 10; i++) {
			check_sample = hcsr04_get_distance();
			sum_values += check_sample;
			
			if (check_sample > min_value && check_sample < max_value)
			{
				legit_sample_counter++;
			}
		}
		
		if (legit_sample_counter > 7) {
			return sum_values / 10;
		}
		return 0;
	}
	
	while (1)
	{
		if (hcsr04_sample_ready()) {
			if (check_samples() != 0) {
				distance_to_move = (check_samples() + distance_from_plattform) - minimum_distance_to_object; // Distance the robot has to move for picking up an object
				break;
			}
			else {
				if (moved_counter < max_moved_times) {
					rotate += rotation_deg;
					
					if (rotate > 360) {
						distance_to_move = step_forward;
						rotate = 0;
						moved_counter++;
						// Use distance_to_move to motor task! (no_dectetion_move = 5.6)
					}
				}
				else {
					// Could not find the object start over the whole processor from start?
				}
			}
		}
	}
	// Use distance_to_move to motor task!
	
	/* Sensor end */

	/* P2 is now over. */
	/*while(1){
		// Find the object and pick it up
		if (hcsr04_sample_ready()){
			printf("Distance: %d\n", hcsr04_get_distance());
		}
	}*/
}

