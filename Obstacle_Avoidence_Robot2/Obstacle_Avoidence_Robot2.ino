#include <Servo.h>

// Ultrasonic sensor pins
#define trigPin 8
#define echoPin 9

// Servo motor pin
#define servoPin 10

// Motor pins (via L293D)
#define motor1Pin1 5
#define motor1Pin2 4
#define motor2Pin1 3
#define motor2Pin2 2

Servo myServo;

void setup() {
  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Motors
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Servo
  myServo.attach(servoPin);
  myServo.write(90); // Start at center

  Serial.begin(9600);
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
  delay(300);
  long centerDist = readDistance();
  Serial.print("Center: ");
  Serial.println(centerDist);

  if (centerDist > 20) {
    moveForward();
  } else {
    stopMotors();
    delay(300);

    myServo.write(0); // Look left
    delay(500);
    long leftDist = readDistance();
    Serial.print("Left: ");
    Serial.println(leftDist);

    myServo.write(180); // Look right
    delay(500);
    long rightDist = readDistance();
    Serial.print("Right: ");
    Serial.println(rightDist);

    if (leftDist > rightDist) {
      turnLeft();
      delay(600);
    } else {
      turnRight();
      delay(600);
    }

    stopMotors();
    delay(300);
  }
}