/*
 * TWI.h
 *
 * Created: 2018-04-23 10:46:36
 *  Author: Ali Hassan
 */ 

#include <asf.h>
#ifndef TWI_H_
#define TWI_H_
#define unoAddress 0x08 //Arduino Uno Adress
#define MegaAddr 0x10	//Arduino Mega Adress
#define grap_object 12
#define release_object 13
#define grip_successfull 10
#define grip_failed 0x11
#define TWI_SUCCESS              0
#define TWI_INVALID_ARGUMENT     1
#define TWI_ARBITRATION_LOST     2
#define TWI_NO_CHIP_FOUND        3
#define TWI_RECEIVE_OVERRUN      4
#define TWI_RECEIVE_NACK         5
#define TWI_SEND_OVERRUN         6
#define TWI_SEND_NACK            7
#define TWI_BUSY                 8
#define TWI_ERROR_TIMEOUT        9
#define TWI_TIMEOUT              30000
#define I2C_SPEED  100000
typedef struct twi_packet {
	uint8_t addr[3];		// TWI address/commands to issue to the other chip (node).
	uint32_t addr_length;	// Length of the TWI data address segment (1-3 bytes).
	void *buffer;			//data to be transferred.
	uint32_t length; 		// How many bytes do we want to transfer.
	uint8_t chip;				//! TWI chip address to communicate with.
} twi_packet_t;

void Twi_master_init(Twi* pTWI);

uint8_t master_write_cmd(Twi* ptwi, uint8_t cmd);
uint8_t master_read_cmd(Twi* p_twi);
uint32_t I2C_mk_addr(const uint8_t *addr, int len);
uint32_t I2C_master_read(Twi *p_twi, twi_packet_t *p_packet);
uint8_t array[10];

#endif /* TWI_H_ */
