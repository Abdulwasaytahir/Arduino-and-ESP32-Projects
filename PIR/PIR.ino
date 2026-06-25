const int pirPin = 2;     // PIR output pin
const int ledPin = 13;    // Built-in LED

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("PIR Sensor Test Ready...");
}

void loop() {
  int motion = digitalRead(pirPin);
  if (motion == HIGH) {
    Serial.println("Motion Detected!");
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("No Motion");
    digitalWrite(ledPin, LOW);
  }
  delay(1000);  // Delay for readability
}
