/*
 * math_functions.h
 *
 * Created: 4/27/2018 10:49:58 AM
 *  Author: Philip Ekholm
 */ 

#pragma once

#include <stdint.h>

double math_get_angle_deg(double rad);
double math_get_angle_rad(double deg);
uint16_t get_euclid_distance(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1);
double math_atan2(double x, double y, double x1, double y1);
