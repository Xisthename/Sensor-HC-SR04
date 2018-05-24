/*
 * delay_driver.h
 *
 * Created: 5/18/2018 11:11:23 AM
 *  Author: Philip Ekholm
 */ 

int delay_driver_init(void);
void delay_microseconds(uint32_t us);
void delay_milliseconds(uint32_t ms);
void delay_seconds(uint32_t s);
