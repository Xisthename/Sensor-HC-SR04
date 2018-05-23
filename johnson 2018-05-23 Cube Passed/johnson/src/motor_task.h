/*
 * motor_task.h
 *
 * Created: 4/23/2018 11:26:25 AM
 *  Author: Philip Ekholm
 */ 

#pragma once

#define TASK_MOTOR_STACK_SIZE (2048/ sizeof(portSTACK_TYPE))
#define TASK_MOTOR_PRIORITY   (2)

#define TASK_DEBUG_MOTOR_PIN  (53)

void motor_task(void *pvParameters);

struct motor_task_instruction{
	int16_t distance;
	double angle;
};

xQueueHandle motor_task_instruction_handle;
