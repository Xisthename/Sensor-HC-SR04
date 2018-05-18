/*
 * positions.c
 *
 * Created: 5/18/2018 8:23:08 AM
 *  Author: Philip Ekholm, Joel Skogsberg, Ali Hassan
 */ 

#include "positions.h"
#include "drivers/TWI.h"

uint8_t array[10]={};
twi_packet_t packet_pos ={
	.addr[0] = 0x00,	// TWI address/commands to issue to the other chip (node).
	.addr [1]=0,
	.addr_length =0,	// Length of the TWI data address segment (1-3 bytes)
	.chip = unoAddress,		// Adress to Mega kort
	.buffer = array, // where to save packet
	.length =10,	//packet length
};

struct point get_pos(void){
	
	struct point coordinates;
	
	coordinates.x = array[8]*2;
	coordinates.y = array[9]*2;
	/* Mock, replace later */
	coordinates.x = 0;
	coordinates.y = 0;
	return coordinates;
};

struct point get_box(void){
	
	struct point coordinates;
	
	coordinates.x = array[0]*2;
	coordinates.y = array[1]*2;
	/* Mock, replace later */
	coordinates.x = 50;
	coordinates.y = 50;
	return coordinates;
};

struct point get_ball(void){
	struct point coordinates;
	
	coordinates.x = array[4]*2;
	coordinates.y = array[5]*2;
	/* Mock, replace later */
	coordinates.x = 300;
	coordinates.y = 300;
	return coordinates;
};

struct point get_cube(void){
	
	struct point coordinates;
	
	coordinates.x = array[2]*2;
	coordinates.y = array[3]*2;
	/* Mock, replace later */
	coordinates.x = 300;
	coordinates.y = 300;
	return coordinates;
};

void update_positions(void){
	I2C_master_read(TWI1, &packet_pos);
}
