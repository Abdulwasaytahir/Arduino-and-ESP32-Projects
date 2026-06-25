#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

/* ===== Motor A (Left) ===== */
#define L_RPWM 26
#define L_LPWM 25

/* ===== Motor B (Right) ===== */
#define R_RPWM 33
#define R_LPWM 32

/* ===== Speed Settings ===== */
int speedStraight = 170;   // 150–170
int speedTurn     = 200;    // 80–100

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_RC_CAR");

  // Attach PWM directly to pins (NEW METHOD)
  ledcAttach(L_RPWM, 20000, 8);
  ledcAttach(L_LPWM, 20000, 8);
  ledcAttach(R_RPWM, 20000, 8);
  ledcAttach(R_LPWM, 20000, 8);

  stopCar();
}

/* ===== Movement Functions ===== */
void forward() {
  ledcWrite(L_RPWM, speedStraight);
  ledcWrite(L_LPWM, 0);
  ledcWrite(R_RPWM, speedStraight);
  ledcWrite(R_LPWM, 0);
}

void backward() {
  ledcWrite(L_RPWM, 0);
  ledcWrite(L_LPWM, speedStraight);
  ledcWrite(R_RPWM, 0);
  ledcWrite(R_LPWM, speedStraight);
}

void rightTurn() {
  // Left motor STOP
  ledcWrite(L_RPWM, 0);
  ledcWrite(L_LPWM, 0);

  // Right motor FORWARD
  ledcWrite(R_RPWM, speedTurn);
  ledcWrite(R_LPWM, 0);
}

void leftTurn() {
  // Right motor STOP
  ledcWrite(R_RPWM, 0);
  ledcWrite(R_LPWM, 0);

  // Left motor FORWARD
  ledcWrite(L_RPWM, speedTurn);
  ledcWrite(L_LPWM, 0);
}


void stopCar() {
  ledcWrite(L_RPWM, 0);
  ledcWrite(L_LPWM, 0);
  ledcWrite(R_RPWM, 0);
  ledcWrite(R_LPWM, 0);
}

/* ===== Main Loop ===== */
void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.println(cmd);

    if      (cmd == 'F') forward();
    else if (cmd == 'B') backward();
    else if (cmd == 'L') leftTurn();
    else if (cmd == 'R') rightTurn();
    else if (cmd == 'S') stopCar();
  }
}