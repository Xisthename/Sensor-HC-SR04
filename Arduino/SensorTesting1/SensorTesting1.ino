/*
  Program for testing a Ultrasonic Sensor HC-SR04
  Author Emil
  Updated by Daniel
*/

// defines pins numbers
const int trigPin = 4;
const int echoPin = 3;

// defines variables
long duration;
int distance;
int distance10;

int distanceArray[10];
int count = 0;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication
}
void loop() {

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  distance10 += distance; // Adds the current distance to distance10
  count++; // Increments a counter

  if (count == 10) { // Count equals 10
    Serial.print("Distance array of 10: ");
    Serial.println(distance10 / 10); // Calculates the average distance from the last 10 samples
    distance10 = 0; // Resets the value of distance10 
    count = 0; // Resets the counter
  }
  delay(100);
}
