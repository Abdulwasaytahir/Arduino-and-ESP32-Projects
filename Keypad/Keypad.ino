
#include <Keypad.h>

// Define LED and buzzer pins
const int redLED = 13;
const int greenLED = 11;
const int blueLED = 12;
const int buzzer = 10;

// Define keypad rows and columns
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Connect keypad row and column pins
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

// Create keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case '3':
        digitalWrite(redLED, HIGH);
        break;
      case '1':
        digitalWrite(greenLED, HIGH);
        break;
      case '2':
        digitalWrite(blueLED, HIGH);
        break;
      case '4':
        digitalWrite(buzzer, HIGH);
        break;
      case '0':
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, LOW);
        digitalWrite(blueLED, LOW);
        digitalWrite(buzzer, LOW);
        break;
    }
  }
}

