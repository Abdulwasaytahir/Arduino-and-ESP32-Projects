// ================= PIN DEFINITIONS =================
#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 21
#define ENA 17
#define ENB 16

#define IR_PIN 4
#define MIC_PIN 34

// ================= VARIABLES =================
unsigned long lastSoundTime = 0;
int clapCount = 0;
int threshold = 600;     // Adjust if needed
bool obstacle = false;

// ================= SETUP =================
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(IR_PIN, INPUT);
  pinMode(MIC_PIN, INPUT);

  stopRobot();
}

// ================= LOOP =================
void loop() {
  obstacle = digitalRead(IR_PIN) == LOW;

  if (obstacle) {
    stopRobot();
    clapCount = 0;
    return;
  }

  int micValue = analogRead(MIC_PIN);

  if (micValue > threshold) {
    clapCount++;
    lastSoundTime = millis();
    delay(200); // debounce
  }

  if (clapCount > 0 && millis() - lastSoundTime > 800) {
    executeCommand(clapCount);
    clapCount = 0;
  }
}

// ================= COMMAND EXECUTION =================
void executeCommand(int count) {
  if (count == 1) {
    moveForward();
  } 
  else if (count == 2) {
    turnLeft();
  } 
  else if (count == 3) {
    turnRight();
  } 
  else {
    stopRobot();
  }
}

// ================= MOTOR FUNCTIONS =================
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
