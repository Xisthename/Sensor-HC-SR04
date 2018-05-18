#include <asf.h>

#include "motor_task.h"
#include "pin_mapper.h"
#include "MotorControll.h"
#include "math_functions.h"
#include "drivers/encoder.h"
#include "drivers/TWI.h"
#include "main_task.h"

#define MOTOR_TASK_PERIODICITY 4 /* The number on the macro will decide the periodicity of the task */

static void forDelay(){
	for(int i = 0;i < 200000;i++){
		drive(1500,1500);
	}
}

void motor_task(void *pvParameters) {
	portTickType xLastWakeTime;
	const portTickType xTimeIncrement = MOTOR_TASK_PERIODICITY;
 	int16_t angle = 0;
 	int16_t distance = 0;
	uint8_t flagg = 0;
	uint8_t flaggu = 0;
	uint8_t accelerate = 200;
	
	struct motor_task_instruction current_instruction;
	
	while(1){
		/* Get current tick count */
		xLastWakeTime = xTaskGetTickCount();
		
		while(!xQueuePeek(motor_task_instruction_handle, &current_instruction, 10));
		
		angle = (int16_t)current_instruction.angle/3.809;
		distance = convertDistance(current_instruction.distance);
		
		if(angle<0){
			angle = angle * -1;
			flagg = 3;
		}else{
			flagg = 0;
		}
		if(get_counterA() < angle + distance && get_counterB() < angle + distance){
			
			
			if(get_counterA() < (angle) && get_counterB() < (angle) && flagg == 0){
				driveVinkel(1);
			}else if(get_counterA() < (angle) && get_counterB() < (angle) && flagg == 3){
				driveVinkel(-1);
			}else{
				if(flaggu != 1){
					forDelay();
					flaggu = 1;
				}
				drive(1753 - accelerate,1793 - accelerate);
				if(accelerate > 0){
					accelerate = accelerate - 1;
					if (accelerate < 0){
						accelerate = 0;
					}
				}
			}
		}
		else{
			forDelay();
			flagg = 1;
			flaggu = 0;
			accelerate = 200;
			/* Finished driving the distance, empty queue for new instruction */
			xQueueReceive(motor_task_instruction_handle, &current_instruction, 10);
			resetCounterA();
			resetCounterB();
		}
	
		/* The task is now done, go to sleep until it's time again */
		vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
	}

}

