#include <Servo.h>

#define trigPin 8
#define echoPin 9
#define servoPin 10

Servo myServo;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(servoPin);
  myServo.write(90); // Center
  delay(1000);
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  // Servo center
  myServo.write(90);
  delay(500);
  Serial.print("Center: ");
  Serial.print(readDistance());
  Serial.println(" cm");

  // Servo left
  myServo.write(0);
  delay(500);
  Serial.print("Left: ");
  Serial.print(readDistance());
  Serial.println(" cm");

  // Servo right
  myServo.write(180);
  delay(500);
  Serial.print("Right: ");
  Serial.print(readDistance());
  Serial.println(" cm");

  delay(1000); // Wait before next loop
}