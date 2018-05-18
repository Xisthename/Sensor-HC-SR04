/*
 * MotorControll.c
 *
 * Created: 2018-04-20 11:40:38
 *  Author: Mohamed & Charif
 */ 
#include <asf.h>
#include <sam3x8e.h>
#include "MotorControll.h"
#include "drivers/encoder.h"
#include "config/PWM_Configuration.h"

int16_t e = 0;
int16_t u = 0;
uint16_t Uv = 0;
uint16_t Uh = 0;
uint8_t k = 2;

 /************************************************************************/
 /* A funktion that gives power to motor A                                                                     */
 /************************************************************************/
 void motorA(uint16_t speed1){
	 pwm_pin_7(speed1);
 }
 
 /************************************************************************/
 /* A funktion that gives power to motor B                                                                     */
 /************************************************************************/
 void motorB(uint16_t speed2){
	 pwm_pin_6(speed2);
 }
 /************************************************************************/
 /* This function converts a specific distance to pulses for the motor                                                                  */
 /************************************************************************/
 uint16_t convertDistance(uint16_t cm){
	return cm/1.396;
 }
 
 /************************************************************************/
 /* This function gives power to both of the motors and keeps them
 /* moving at he same speed
 /************************************************************************/
 void drive(uint16_t U0v, uint16_t U0h){
	 e = (get_counterA() - get_counterB());
	 u = (k*e);
	 
	 Uv = U0v - u;
	 Uh = u + U0h;
	 
	 motorA(Uv);
	 motorB(Uh);
	 
// 	 printf("encoder A %d och encoder B %d\n",get_counterA(), get_counterB());
// 	 printf("Motor A %d och motor B %d\n",Uv,Uh);
//	 printf("e = %d\n",e);
//	 printf("Fram=============\n");
 }
 
 /************************************************************************/
 /* This function rotates the robot and keeps it in its place, one motor */
 /* rotates forward the other motor rotates backwards                    */
 /************************************************************************/
 void driveVinkel(int riktning){
	 if(riktning == 1){
		 drive(1400, 1575);
	 }else if (riktning == -1){
		 drive(1575, 1400);
	 }else{
		 drive(1500, 1500);
	 }
 }
  