/*
 * PWM_Configuration.h
 *
 * Created: 2018-05-02 12:10:34
 *  Author: ziko205i
 */ 


#ifndef PWM_CONFIGURATION_H_
#define PWM_CONFIGURATION_H_

void PWM_init(void);
void initPin6(void);
void initPin7(void);
void pwm_pin_6(uint32_t duty);
void pwm_pin_7(uint32_t duty);



#endif /* PWM_CONFIGURATION_H_ */