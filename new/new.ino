#include <SoftwareSerial.h>

// Define the Bluetooth module
SoftwareSerial bluetooth(0, 1);  // RX, TX

// Motor Control Pins for L298N
int in1 = 9;
int in2 = 8;
int in3 = 7;
int in4 = 6;

void setup() {
  // Initialize motor control pins as outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initialize Serial Monitor and Bluetooth
  Serial.begin(9600);
  bluetooth.begin(9600);
  Serial.println("Bluetooth Voice-Controlled Car with L298N");
}

void loop() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    Serial.println(command);

    // Forward
    if (command == 'F') {
      forward();
    }
    // Backward
    else if (command == 'B') {
      backward();
    }
    // Left
    else if (command == 'L') {
      left();
    }
    // Right
    else if (command == 'R') {
      right();
    }
    // Stop
    else if (command == 'S') {
      stopCar();
    }
  }
}

// Function to move forward
void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Function to move backward
void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

// Function to turn left
void left() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// Function to turn right
void right() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Function to stop the car
void stopCar() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}