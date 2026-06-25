#include <Servo.h>

#define trigPin 8
#define echoPin 9

#define motor1Pin1 5
#define motor1Pin2 4
#define motor2Pin1 3
#define motor2Pin2 2

#define servoPin 10

Servo myServo;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  myServo.attach(servoPin);
  myServo.write(90); // Center position

  Serial.begin(9600);
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // in cm
}

void moveForward() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void turnLeft() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void turnRight() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void stopMotors() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

void loop() {
  myServo.write(90); // Center
  delay(500);
  long centerDist = readDistance();

  if (centerDist > 20) {
    moveForward();
  } else {
    stopMotors();
    delay(300);

    myServo.write(0); // Left
    delay(500);
    long leftDist = readDistance();

    myServo.write(180); // Right
    delay(500);
    long rightDist = readDistance();

    if (leftDist > rightDist) {
      turnLeft();
      delay(600);
    } else {
      turnRight();
      delay(600);
    }
  }

  delay(100);
}