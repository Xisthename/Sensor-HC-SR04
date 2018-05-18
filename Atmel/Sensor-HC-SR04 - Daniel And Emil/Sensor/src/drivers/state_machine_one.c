/*
 * state_machine.c
 *
 * Created: 2018-05-16 10:09:57
 * Author: Emil
 * Updated by Daniel 2018-05-17 (not done with the update)
 */ 

/*Enum construct declares states */
typedef enum { 
	READ_SENSOR,
	SENSOR_DETECTION,
	NO_DETECTION,
	GRIP_OBJECT,
	CHECK_OBJECT
} state_t;

state_t current_state;
state_t next_state;

int detectSteelBallOrBox = 0; // What object should be detected? 0 = SteelBall, 1 = Box
int dist_middle_sensor;
int rot_counter = 0;
int mov_counter = 0;

 int distanceFromPlattform = 6; // The sensor is located 6 cm in front of the robot

 int gripSteelBallDistance = 20; // The distance in front of the platform to the steel ball for picking it up
 int gripBoxDistance = 25; // The distance in front of the platform to the ball for picking it up
 int reliableDistance = 10;

 int gripDistance; // What distance in front of the platform will an object be picked up from

 int dist_min = 5; // Minimum distance for detecting an object
 int dist_max = 40; // Maximum distance for detecting an object

 int dist_min_ball_interval; // Minimum interval for the steel ball
 int dist_max_ball_interval; // Maximum interval for the steel ball
 int dist_min_box_interval; // Minimum interval for the box
 int dist_max_box_interval; // Maximum interval for the box

int dist_min_current_interval; // Current minimum interval that is used which is different regarding the object
int dist_max_current_interval; // Current maximum interval that is used which is different regarding the object

int distanceToMove; // The distance that the robot has to move to be able to pick up an object

void SetupIntervalls() {
	dist_min_ball_interval = dist_min; // Sets the minimum interval for the steel ball
	dist_max_ball_interval = 18; // Sets the maximum interval for the steel ball
	dist_min_box_interval = dist_min; // Sets the minimum interval for the box
	dist_max_box_interval = dist_max; // Sets the maximum interval for the box
		
	if (detectSteelBallOrBox == 0) {
		// Values for the steel ball
		dist_min_current_interval = dist_min_ball_interval;
		dist_max_current_interval = dist_max_ball_interval;
		gripDistance = gripSteelBallDistance;
	}
	else {
		// Values for the box
		dist_min_current_interval = dist_min_box_interval;
		dist_max_current_interval = dist_max_box_interval;
		gripDistance = gripBoxDistance;
	}
}

int checkSamples = 0; // TEMP

int SensorMain(int *found) {
	current_state = READ_SENSOR; // Sets the current state to read
	*found = -1;
	
	
	if (checkSamples != 0) {
		distanceToMove = (checkSamples + distanceFromPlattform) - gripDistance;
	}
	
	
	
	
	
		switch (current_state) {
			case READ_SENSOR:
				if (checkSamples != 0) {
					//The object is within range of minimum and maximum distance
					*found = 1; // The object has been found
					next_state = SENSOR_DETECTION; // Switch state to detection
					break;
				}
				next_state = NO_DETECTION; // Otherwise no object was detected and therefore switch state to no detection
				break;
			case SENSOR_DETECTION:
				distanceToMove = (dist_middle_sensor + distanceFromPlattform) - gripDistance;
				next_state = GRIP_OBJECT;
				break;
			case NO_DETECTION:
				rot_counter = rot_counter + 1;
		
				if (rot_counter >= 8)
				{
					//kör framåt 5,6 cm
					mov_counter = mov_counter + 1;
				if (mov_counter < 5)
					{
						//GG
						//Kör och lämna luft i lådan
					}
				}
				break;
			case GRIP_OBJECT:
				*found = 1;
				next_state = CHECK_OBJECT;
				//pausa taskena
	
				break;
			case CHECK_OBJECT:
				//Implementeras i mån av tid
				next_state = READ_SENSOR;
				break;
		}
		current_state = next_state;
}