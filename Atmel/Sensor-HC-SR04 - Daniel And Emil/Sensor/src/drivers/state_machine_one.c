/*
 * state_machine.c
 *
 * Created: 2018-05-16 10:09:57
 *  Author: Emil
 */ 

/*Enum construct declares states */
enum { 
	READ_SENSOR,
	MIDDLE_SENSOR,
	NO_DETECTION,
	GRIP_OBJECT,
	CHECK_OBJECT
} current_state;

int dist_middle_sensor = 0;
int rot_counter = 0;
int mov_counter = 0;
int dist_min_ball = 14;
int dist_max_ball = 16;
int dist_min_box = 19;
int dist_max_box = 21;



current_state = READ_SENSOR;

switch (current_state) {
	
	case READ_SENSOR:
	if(dist_middle_sensor > 0 && dist_middle_sensor > 50) {
	current_state= MIDDLE_SENSOR;
	}
	else {
		current_state = NO_DETECTION;
	}
	break;
	
	case MIDDLE_SENSOR :
	
	if(dist_middle_sensor >= dist_min && dist_middle_sensor =< dist_max) {
		current_state = GRIP_OBJECT;
	}	
	else if (dist_middle_sensor < 14) {
		//K�r fram 5,6 cm
		current_state = READ_SENSOR;
	} 
	else if (dist_middle_sensor < 18 ) {
		//Backa 5,6 cm
		current_state = READ_SENSOR;
	} 
	break;
		
	case NO_DETECTION :
	//rotera X grader
	rot_counter = rot_counter + 1;
	if (rot_counter >= 8)
	{
		//k�r fram�t 5,6 cm
		mov_counter = mov_counter + 1;
	if (mov_counter =< 5)
		{
			//GG
			//K�r och l�mna luft i l�dan
		}
	}
	break;
	
	case GRIP_OBJECT :
	//S�tt en flagga att man �r r�tt positionerad.
	current_state = CHECK_OBJECT;
	//pausa tasken
	
	break;
	
	//State f�r att kolla om objektet ligger kvar efter uppplockning
	case CHECK_OBJECT :
	//Implementeras i m�n av tid
	current_state = READ_SENSOR;
	break;
