/*
 * MotorControll.h
 *
 * Created: 2018-04-20 11:40:58
 *  Author: Mohamed & Charif
 */ 


#ifndef MOTORCONTROLL_H_
#define MOTORCONTROLL_H_

void motorA(uint16_t speed1);

void motorB(uint16_t speed2);

void drive(uint16_t U0v, uint16_t U0h);

// int getHoger();
// 
// int getVenster();

void driveVinkel(int riktning);

uint16_t convertDistance(uint16_t cm);

#endif /* MOTORCONTROLL_H_ */