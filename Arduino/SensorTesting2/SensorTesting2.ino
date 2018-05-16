/*
  Program for testing two Ultrasonic Sensors HC-SR04 at the same time
  Author Emil
  Updated by Daniel
*/

// defines pins numbers
const int trigPinLeft = 2;
const int echoPinLeft = 3;

const int trigPinRight = 6;
const int echoPinRight = 7;

// defines variables
long durationLeft;
int distanceLeft;
int distance10Left;

long durationRight;
int distanceRight;
int distance10Right;

int countLeft = 0;
int countMiddle = 0;
int countRight = 0;

void setup() {
  pinMode(trigPinLeft, OUTPUT); // Sets the trigPin as an Output
  pinMode(trigPinRight, OUTPUT); // Sets the trigPin as an Output

  pinMode(echoPinLeft, INPUT); // Sets the echoPin as an Input
  pinMode(echoPinRight, INPUT); // Sets the echoPin as an Input

  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  digitalWrite(trigPinLeft, LOW); // Clears the trigPin
  delayMicroseconds(2);
  digitalWrite(trigPinLeft, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds which starts the left sensor
  delayMicroseconds(10);
  digitalWrite(trigPinLeft, LOW);
  durationLeft = pulseIn(echoPinLeft, HIGH); // Reads and stores the sound wave travel time in microseconds into durationLeft
  distanceLeft = durationLeft * 0.034 / 2; // Calculates the distance with durationLeft and the sound of speed in air
  /*Serial.print("Distance Left: ");
    Serial.println(distanceLeft);*/ // Prints out the current distance from the left sensor

  digitalWrite(trigPinRight, LOW); // Clears the trigPin
  delayMicroseconds(2);
  digitalWrite(trigPinRight, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds which starts the right sensor
  delayMicroseconds(10);
  digitalWrite(trigPinRight, LOW);
  durationRight = pulseIn(echoPinRight, HIGH); // Reads and stores the sound wave travel time in microseconds into durationRight
  distanceRight = durationRight * 0.034 / 2; // Calculates the distance with durationRight and the sound of speed in air
  /*Serial.print("Distance Right: ");
    Serial.println(distanceRight);*/ // Prints out the current distance from the right sensor

  distance10Left += distanceLeft; // Adds the current distance from the left sensor into distance10Left
  countLeft++; // Increments a counter

  if (countLeft == 10) { // Counter equals 10
    Serial.print("Distance Left Array: ");
    Serial.println(distance10Left / 10); // Calculates and prints out the left sensor's average distance from the last 10 samples
    distance10Left = 0; // Resets the value
    countLeft = 0; // Resets the counter
  }


  distance10Right += distanceRight; // Adds the current distance from the right sensor into distance10Right
  countRight++; // Increments a counter

  if (countRight == 10) { // Counter equals 10
    Serial.print("Distance Right Array: ");
    Serial.println(distance10Right / 10); // Calculates and prints out the right sensor's average distance from the last 10 samples
    distance10Right = 0; // Resets the value
    countRight = 0; // Resets the counter
  }
  delay(100);
}
