/*
 * TWI.c
 *
 * Created: 2018-04-23 10:45:56
 *  Author: Ali Hassan
 */ 
#include <asf.h>
#include "TWI.h"
/* this function initiates the master Due
* TWI1 are used for the master
* enables TWI pios
*/
	 
void Twi_master_init(Twi* pTWI){
	pmc_enable_periph_clk(ID_TWI1); /* power the clock for the TWI with pmc */
	PIOB->PIO_PDR|= (PIO_PB12)|(PIO_PB13);	/* Enable TWI pios */
	//PIOA->PIO_PDR|= (PIO_PA18)|(PIO_PA17);	/* Enable TWI pios */
	pTWI->TWI_CR = (0x1u << 7);				// TWIn software reset
	pTWI->TWI_RHR;							// Flush reception buffer
	/* Set Master Disable bit and Slave Disable bit */
	pTWI->TWI_CR = TWI_CR_MSDIS;
	pTWI->TWI_CR = TWI_CR_SVDIS;
	pTWI->TWI_CR |= (0x1u << 2);			/* Set Master Enable bit */
	pTWI->TWI_MMR |= TWI_MMR_DADR(unoAddress);//device address
	pTWI->TWI_PTCR |= (0x1u << 0)|(0x1u << 8);//Receiver/transmitter Transfer Enable
	pTWI->TWI_IDR = ~0UL;					/* Disable TWI interrupts */
	pTWI->TWI_CWGR |= I2C_SPEED;
}
/*this function startar a connection between a master and especific
* slave with unique ID. this function used for earlier experiments
*/
void twi_Start(Twi* pTWI, uint8_t slave_address, uint8_t R_W) { //read=1, write=0
	//set slave address
	pTWI->TWI_MMR = (pTWI->TWI_MMR & ~TWI_MMR_DADR_Msk)|TWI_MMR_DADR(slave_address);
	//set read/write direction
	if (R_W == 0) { //write
		pTWI->TWI_MMR &= ~(0x1u << 12);//Master write Direction 
	}
	else if (R_W == 1) { //read
		pTWI->TWI_MMR |= (0x1u << 12); //Master Read Direction 
	}
	pTWI->TWI_CR |= (0x1u << 0);//send start condition
	while (!(pTWI->TWI_SR & TWI_SR_TXRDY));//wait for ack
}

/*this function writes a commando to the slave
*returns a cmd send to the slave
* param p_wti: pointer to TWi interface
*param cmd: commando to send
*/
uint8_t master_write_cmd(Twi* ptwi, uint8_t cmd){
	uint8_t cmd_send =0;
	/* Set write mode, slave address and 3 internal address byte lengths */
	ptwi->TWI_MMR = 0;
	ptwi->TWI_MMR = TWI_MMR_DADR(unoAddress) |
	((0 << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);
 	
	 while (!(ptwi->TWI_SR & TWI_SR_TXRDY));	
	ptwi->TWI_THR =cmd;//write commando to THR
	
	ptwi->TWI_CR = TWI_CR_STOP;
	while (!(ptwi->TWI_SR & TWI_SR_TXCOMP)){}
	cmd_send=cmd;
	return cmd_send;
}
/*this function reads a commando from the slave
*returns a cmd sent by the slave
* param p_wti: pointer to TWi interface
*/
uint8_t master_read_cmd(Twi* p_twi){
	uint8_t cmd;
	/* Set read mode, slave address and 3 internal address byte lengths */
	p_twi->TWI_MMR = 0;
	p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(unoAddress) |
	((0<< TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);
	p_twi->TWI_CR = TWI_CR_START;
	while (!(TWI1 ->TWI_SR & TWI_SR_RXRDY)){}
	cmd =TWI1->TWI_RHR; 
	p_twi->TWI_CR = TWI_CR_STOP;
	while (!(p_twi->TWI_SR & TWI_SR_TXCOMP)) {}
	p_twi->TWI_SR;
	return cmd;

}
/*this function read a packet from the positionsystem
* and saves the packet to un array 
*/
uint32_t I2C_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
	printf("welcome\n");
	uint32_t status;
	uint32_t cnt = p_packet->length;
	uint8_t *buffer = p_packet->buffer;
	uint8_t stop_sent = 0;
	uint32_t timeout = TWI_TIMEOUT;;
	
	/* Check argument */
	if (cnt == 0) {
		return TWI_INVALID_ARGUMENT;
	}

	/* Set read mode, slave address and 3 internal address byte lengths */
	p_twi->TWI_MMR = 0;
	p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) |
	((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) &
	TWI_MMR_IADRSZ_Msk);

	/* Set internal address for remote chip */
	p_twi->TWI_IADR = 0;
	p_twi->TWI_IADR = I2C_mk_addr(p_packet->addr, p_packet->addr_length);

	/* Send a START condition */
	if (cnt == 1) {
		p_twi->TWI_CR = TWI_CR_START | TWI_CR_STOP;
		stop_sent = 1;
		} else {
		p_twi->TWI_CR = TWI_CR_START;
		stop_sent = 0;
	}

	while (cnt > 0) {
		status = p_twi->TWI_SR;
		if (status & TWI_SR_NACK) {
			return TWI_RECEIVE_NACK;
		}

		if (!timeout--) {
			return TWI_ERROR_TIMEOUT;
		}
		
		/* Last byte ? */
		if (cnt == 1  && !stop_sent) {
			p_twi->TWI_CR = TWI_CR_STOP;
			stop_sent = 1;
		}

		if (!(status & TWI_SR_RXRDY)) {
			continue;
		}
		*buffer++ = p_twi->TWI_RHR;

		cnt--;
		timeout = TWI_TIMEOUT;
	}

	while (!(p_twi->TWI_SR & TWI_SR_TXCOMP)) {
	}

	p_twi->TWI_SR;

	return TWI_SUCCESS;
}

 /* The TWI module address register is sent out MSB first. 
 *And the size controls which byte is the MSB to start with.
 *
 */
 uint32_t I2C_mk_addr(const uint8_t *addr, int len)
 {
	uint32_t val;

	if (len == 0)
	return 0;

	val = addr[0];
	if (len > 1) {
		val <<= 8;
		val |= addr[1];
	}
	if (len > 2) {
		val <<= 8;
		val |= addr[2];
	}
	return val;
 }
