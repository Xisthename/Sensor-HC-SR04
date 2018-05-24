/*
 * nav_task.c
 *
 * Created: 4/23/2018 11:35:40 AM
 * Author:  Philip Ekholm
 * Updated: Ali Hassan
 * Updated: Daniel -> Sensor: Movement and rotation logic (2018-05-18) And updated again (2018-05-22)
 * Updated: Emil -> Sensor: Filtering data samples (2018-05-18) // Done
 */ 
#include <asf.h>
#include <stdio.h>

#include "main_task.h"
#include "motor_task.h"
#include "pin_mapper.h"
#include "drivers/encoder.h"
#include "math_functions.h"
#include "positions.h"
#include "drivers/hcsr04.h"
#include "drivers/TWI.h"

#define USE_NAV_CORRECTION       (1)
#define USE_SENSOR_DETECTION     (0)
#define USE_BOX_NAV              (1)
#define MINIMUM_CORRECTION_ANGLE (1.00)

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
	int16_t distance, minimum_distance_to_object, platform_distance_to_object; /* In cm */
	double alpha, beta, correction_angle; /* In degrees */
	uint8_t mock_pos_index = 0;
	
	printf("------------Welcome------------\n");
	
	/* Read first time package and set static coordinates */
	update_positions();
	current_pos = get_pos();
	box = get_box();
	
	if (ioport_get_pin_level(pin_mapper(SWITCH_CURIE_NOETHER_PIN))) {
		// Marie Curie
		object = get_cube();
		minimum_distance_to_object = 47;
		platform_distance_to_object = 25; // Sensor do not touch!
		printf("Curie selected\n");
	}
	else {
		 // Emmy Noether
		object = get_ball();
		minimum_distance_to_object = 42;
		platform_distance_to_object = 22; // Sensor do not touch!
		printf("Noether selected\n");
	}
	
	vTaskDelay(5000);
		
	update_positions();
	current_pos = get_pos();

	distance = get_euclid_distance(object.x, object.y, current_pos.x, current_pos.y);
	#if USE_NAV_CORRECTION
		distance = distance / 2;
	#else
		distance -= minimum_distance_to_object;
	#endif
	alpha = math_get_angle_deg(math_atan2(object.x, object.y, current_pos.x, current_pos.y));
		
	struct motor_task_instruction inst = {
		.distance = distance,
		.angle = alpha
	};
		
	xQueueSend(motor_task_instruction_handle, &inst, 10);
	earlier_pos = current_pos;
	
	printf("Current pos: (%d, %d)\n", current_pos.x, current_pos.y);
	printf("Position of object: (%d, %d)\n", object.x, object.y);
	printf("First run, angle: %d, d: %d\n", (int16_t)alpha, inst.distance);

	while(USE_NAV_CORRECTION && (distance - minimum_distance_to_object) > 10) {
		/* Wait for motor task to complete */
		while(xQueuePeek(motor_task_instruction_handle, &inst, 2));

		update_positions();
		current_pos = get_pos();
		printf("Current pos: (%d, %d)\n", current_pos.x, current_pos.y);
		printf("Earlier pos: (%d, %d)\n", earlier_pos.x, earlier_pos.y);
		
		distance = get_euclid_distance(object.x, object.y, current_pos.x, current_pos.y);
		/* Drive 70 % of target distance */
		distance = distance / 2;
		
		if (distance - minimum_distance_to_object < 10) {
			distance = 15;
		}

		/* Calculate the actual angle that was driven */
		beta = math_get_angle_deg(math_atan2(current_pos.x, current_pos.y, earlier_pos.x, earlier_pos.y));
		alpha = math_get_angle_deg(math_atan2(object.x, object.y, current_pos.x, current_pos.y));
		correction_angle = alpha - beta;
		
		printf("correction: %d, d: %d\n", (int16_t)correction_angle, distance);
		printf("Object: (%d, %d)\n", object.x, object.y);

		if (abs(correction_angle) < MINIMUM_CORRECTION_ANGLE){
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

	while(xQueuePeek(motor_task_instruction_handle, &inst, 2));
	
	/* Sensor begin */
	int16_t distance_to_move; // Distance the robot has to move for picking up an object
	const int step_forward = 3; // How much the robot will move forward in attempt to find an object
	const int distance_from_plattform = 7; // The sensor is located 7 cm in front of the robot
	int searching_for_objcet = 1; // A flag for running the while loop
	int obj_distance; // The distance from the sensor to the object
	
	float rotate_tot = 0; // How much has the robot rotated in degree
	float current_rotation = 0; // How much the robot will rotate in degree
	const float rotation_deg = 5; // The motor task can only rotate 3.8 degree as minimum
	const float max_rotation_deg = 30; // How much the robot will turn right and left in attempt to find an object
	int moved_counter = 0; // How many times the robot will move forward when it doesn't find an object
	const int max_moved_times = 3; // Max times the robot will move forward in attempt to find an object
	int turn_right = 1; // Turn right = 1, Turn left = 0
	int step_forward_flag = 0; // Used to drive forward
	const int detection_times = 3; // Try 3 times per rotation to get reliable data
	
	while (USE_SENSOR_DETECTION && searching_for_objcet) {
		/* Try x times per rotation to get reliable data */
		for (int try = 0; try < detection_times; try++) {
			obj_distance = hcsr04_get_distance_filtered();
			
			if (obj_distance != -1) {
				break; // Got reliable data from the sensor, no further try needed
			}
		}
		
		if (obj_distance != -1) {
			/* Found object, the distance the robot has to move for picking up an object and stop searching for the object */
			distance_to_move = (obj_distance + distance_from_plattform) - platform_distance_to_object;
			printf("Correlate: %d\n", distance_to_move);
			searching_for_objcet = 0;
		}
		else {
			if (moved_counter < max_moved_times) {
				if (step_forward_flag == 0) {
					if (turn_right == 1) { // Turn right (1) or turn left (0)
						rotate_tot += rotation_deg; // Store how much the robot has rotated overall
						current_rotation = rotation_deg; // How much the robot will rotate
						printf("Not found: Rotate right\n");
						
						if (rotate_tot > max_rotation_deg) { // The robot has rotated 30 degree to the right (rotate_tot = 35 but the robot have rotated 30)
							turn_right = 0; // Turn left next time
							current_rotation = -max_rotation_deg; // Rotate back to the middle
							rotate_tot = 0; // Rotate to the middle again therefore before and after all rotations the robot is back where it started the rotation part
							printf("Not found: rotate to middle");
						}
					}
					else {
						rotate_tot -= rotation_deg; // Store how much the robot has rotated overall
						current_rotation = -rotation_deg;
						printf("Not found: Rotate left\n");
						
						if (rotate_tot < -max_rotation_deg) {
							current_rotation = max_rotation_deg; // Rotate to the middle after all rotations
							rotate_tot = 0; // After all rotations the robot is in the middle again and therefore the robot has not changed it's angle
							step_forward_flag = 1;
							turn_right = 1;
							printf("Not found: rotate to middle");
						}
					}
				}
				else {
					printf("Not found move forward: " + step_forward);
					distance_to_move = step_forward;
					moved_counter++;
					step_forward_flag = 0;
				}
			}
			else {
				printf("Could not find the object\n");
				searching_for_objcet = 0;
			}
		}
		inst.angle = current_rotation;
		inst.distance = distance_to_move;
		current_rotation = 0;
		distance_to_move = 0;
		
		xQueueSend(motor_task_instruction_handle, &inst, 2);
		while(xQueuePeek(motor_task_instruction_handle, &inst, 2));
	}

	printf("Attempting to grab object\n");
	/* Send a message to the arm that we've found it */
	master_write_cmd(TWI1,grap_object);
	/* Do a busy wait for message back */
	vTaskDelay(9000);
	
	/*Drive over the thing*/
	inst.distance = 50;
	inst.angle = 0;
	xQueueSend(motor_task_instruction_handle, &inst, 2);
	while(xQueuePeek(motor_task_instruction_handle, &inst, 2));
	
	vTaskDelay(5000);

	/* Finally, take us to the box */
	update_positions();
	current_pos = get_pos();
		
	alpha = math_get_angle_deg(math_atan2(box.x, box.y - 100, current_pos.x, current_pos.y));
	beta = math_get_angle_deg(math_atan2(current_pos.x, current_pos.y, earlier_pos.x, earlier_pos.y));
		
	distance = get_euclid_distance(box.x, box.y - 100, current_pos.x, current_pos.y);
	/* Compensate for earlier rotations as well */
	correction_angle = alpha - beta - rotate_tot;
	
	printf("correction: %d, d: %d\n", (int16_t)correction_angle, distance);
	printf("Box: (%d, %d)\n", box.x, box.y);
	
	if (abs(correction_angle) < MINIMUM_CORRECTION_ANGLE){
		correction_angle = 0;
	}
	
	inst.distance = distance;
	inst.angle = correction_angle;

	if (xQueueSend(motor_task_instruction_handle, &inst, 5)) {
		/* Instruction successfully sent to motor task */
		earlier_pos = current_pos;
	}
	
	while(xQueuePeek(motor_task_instruction_handle, &inst, 2));
	
	update_positions();
	current_pos = get_pos();
	
	alpha = math_get_angle_deg(math_atan2(box.x, box.y - 35, current_pos.x, current_pos.y));
	beta = math_get_angle_deg(math_atan2(current_pos.x, current_pos.y, earlier_pos.x, earlier_pos.y));
	
	distance = get_euclid_distance(box.x, box.y - 35, current_pos.x, current_pos.y);
	correction_angle = alpha - beta;

	if (abs(correction_angle) < MINIMUM_CORRECTION_ANGLE) {
		correction_angle = 0;
	}
		
	inst.distance = distance;
	inst.angle = correction_angle;

	if (xQueueSend(motor_task_instruction_handle, &inst, 5)) {
		/* Instruction successfully sent to motor task */
		earlier_pos = current_pos;
	}
		
	while(xQueuePeek(motor_task_instruction_handle, &inst, 2));
	
	master_write_cmd(TWI1,release_object);
	printf("Released object\n");
	/* P2 is now over. */
	printf("--------------End--------------\n");
	while(1);
}

