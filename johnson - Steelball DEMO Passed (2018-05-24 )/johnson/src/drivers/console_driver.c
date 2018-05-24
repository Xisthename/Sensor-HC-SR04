/*
 * console_driver.c
 *
 * Created: 2018-04-21 12:19:49
 *  Author: Uek, Philip Ekholm
 */ 

#include <stdio_serial.h>
#include <asf.h>
#include "conf_board.h"
#include "console_driver.h"

void console_init(void)
/* Enables feedback through the USB-cable back to terminal within Atmel Studio */
/* Note that  the baudrate, parity and other parameters must be set in conf/conf_uart_serial.h */
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}