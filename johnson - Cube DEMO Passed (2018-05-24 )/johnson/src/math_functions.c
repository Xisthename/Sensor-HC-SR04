/*
 * math_functions.c
 *
 * Created: 4/27/2018 10:48:11 AM
 *  Author: Philip Ekholm
 */ 

#include <fastmath.h>

#include "math_functions.h"

#include <stdio.h>
#define PI 3.141592653589

double X = 0;
double Y = 0;
/* Takes an angle in radians and converts to degrees (from 0 to 360) */
double math_get_angle_deg(double rad){
	return (rad * (180/PI));
}

/* Takes an angle in degrees and converts to radians (from 0 to 2 * PI) */
double math_get_angle_rad(double deg){
	return (deg * (PI/180));
}

/* Get euclidean distance between two points (see Pythagorean theorem) */
uint16_t get_euclid_distance(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1) {
	X = x - x1;
	Y = y - y1;
	return sqrt((X*X) + (Y*Y));
}

/* Return the angle in radians, see atan2 Wikipedia for reference */
double math_atan2(double x, double y, double x1, double y1) {
	X = x - x1;
	Y = y - y1;
	if (X == 0 && Y == 0){
		return 0;
	}

	if (X > 0)
		return atan(Y/X);
	else if (X < 0 && Y >= 0)
		return (atan(Y/X) + PI);
	else if (X < 0 && Y < 0)
		return (atan(Y/X) - PI);
	else if (X == 0 && Y > 0)
		return (PI/2);
	else
		return (-PI/2);
}
