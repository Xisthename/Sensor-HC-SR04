/*
 * positions.h
 *
 * Created: 5/18/2018 8:23:48 AM
 *  Author: Philip Ekholm
 */ 

#pragma once

#include <stdint.h>

struct point {
	uint16_t x;
	uint16_t y;
};

struct point get_pos(void);
struct point get_box(void);
struct point get_ball(void);
struct point get_cube(void);
void update_positions(void);
