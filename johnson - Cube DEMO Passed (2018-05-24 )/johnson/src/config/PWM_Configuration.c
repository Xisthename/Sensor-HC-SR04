/*
 * PWM_Configuration.c
 *
 * Created: 2018-05-02 12:10:04
 *  Author: Charif
 */ 
 #include "asf.h"
 #include "PWM_Configuration.h"
 #include "pin_mapper.h"
 

  #define PIN_24  IOPORT_CREATE_PIN(PIOC, 24)  // pinne 6
  #define PIN_23  IOPORT_CREATE_PIN(PIOC, 23)  // pinne 7

   pwm_channel_t PWM_pin_6;
   pwm_channel_t  PWM_pin_7;
  

   void PWM_init(void){
	   pmc_enable_periph_clk(ID_PWM);
	   pwm_channel_disable(PWM,PWM_CHANNEL_7);
	   pwm_channel_disable(PWM,PWM_CHANNEL_6);

	   pwm_clock_t PWMDAC_clock_config = {
		   .ul_clka = 1000000,
		   .ul_clkb = 0,
		   .ul_mck = sysclk_get_cpu_hz()
	   };
	   
	   pwm_init(PWM, &PWMDAC_clock_config);
	   initPin6();
	   initPin7();

   }
   
   void initPin6 (void){
	   pio_configure_pin(PIN_24, PIO_TYPE_PIO_PERIPH_B);
	   PWM_pin_6.channel = PWM_CHANNEL_7;
	   PWM_pin_6.alignment = PWM_ALIGN_LEFT;
	   PWM_pin_6.polarity = PWM_LOW;
	   PWM_pin_6.ul_prescaler = PWM_CMR_CPRE_CLKA;
	   PWM_pin_6.ul_period = 7500;
	   PWM_pin_6.ul_duty = 0;	   
	   pwm_channel_init(PWM, &PWM_pin_6);
	   pwm_channel_enable(PWM, PWM_CHANNEL_7);

   }
   void initPin7 (void){
	   pio_configure_pin(PIN_23, PIO_TYPE_PIO_PERIPH_B);
	   PWM_pin_7.alignment = PWM_ALIGN_LEFT;
	   PWM_pin_7.polarity = PWM_LOW;
	   PWM_pin_7.ul_prescaler = PWM_CMR_CPRE_CLKA;
	   PWM_pin_7.ul_period = 7500;
	   PWM_pin_7.ul_duty = 0;
	   PWM_pin_7.channel = PWM_CHANNEL_6 ;
	   pwm_channel_init(PWM, &PWM_pin_7);
	   pwm_channel_enable(PWM, PWM_CHANNEL_6);
   }
   
   
   void pwm_pin_6(uint32_t speed1){
	    if(speed1<800){
		    speed1=800;
	    }
	    else if(speed1>2200){
		    speed1=2200;
	    }
     pwm_channel_update_duty(PWM, &PWM_pin_6, speed1); 
}

  void pwm_pin_7(uint32_t speed2){
	if(speed2<800){
		speed2=800;
	}
	else if(speed2>2200){
		speed2=2200;
	}
	pwm_channel_update_duty(PWM, & PWM_pin_7, speed2 );
}
