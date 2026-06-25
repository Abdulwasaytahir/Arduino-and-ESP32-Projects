#define BLYNK_TEMPLATE_ID "TMPL66od83gM5"
#define BLYNK_TEMPLATE_NAME "Weather Monitoring System"
#define BLYNK_AUTH_TOKEN "qgXkf1iNSWFVB3dMUf7HMicciUqziQjH"

// Include the library files
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SFE_BMP180.h>

#define TH 5
#define Rain 36

// Create variables for pressure and temperature
double T, P;
char status;

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create an object for the BMP180 sensor
SFE_BMP180 bmp;

DHT dht(TH, DHT11);
BlynkTimer timer;

// Enter your Auth token
char auth[] = "qgXkf1iNSWFVB3dMUf7HMicciUqziQjH";

// Enter your WIFI SSID and password
char ssid[] = "home";
char pass[] = "6strings";

void setup() {
    // Debug console
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);
    if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP sensor, check wiring!");
        while (1); // Stop if sensor is not found
    }
    dht.begin();
    lcd.init();
    lcd.backlight();
    pinMode(Rain, INPUT);
    analogReadResolution(12);

    lcd.setCursor(0, 0);
    lcd.print("System");
    lcd.setCursor(4, 1);
    lcd.print("Loading..");
    delay(4000);
    lcd.clear();
}

// Get the DHT11 sensor values
void DHT11sensor() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);

    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(t);

    lcd.setCursor(8, 0);
    lcd.print("H:");
    lcd.print(h);
}

// Get the rain sensor values
void rainSensor() {
    int Rvalue = analogRead(Rain);
    Rvalue = map(Rvalue, 0, 4095, 0, 100);
    Rvalue = (Rvalue - 100) * -1;
    Blynk.virtualWrite(V2, Rvalue);

    lcd.setCursor(0, 1);
    lcd.print("R:");
    lcd.print(Rvalue);
    lcd.print(" ");
    Serial.println(Rvalue);
}

// Get the pressure values
void pressure() {
    status = bmp.startTemperature();
    if (status != 0) {
        delay(status);
        status = bmp.getTemperature(T);
        if (status != 0) { // Ensure temperature was read successfully
            status = bmp.startPressure(3); // Oversampling setting
            if (status != 0) {
                delay(status);
                status = bmp.getPressure(P, T);
                if (status != 0) {
                    Blynk.virtualWrite(V3, P / 100.0); // Convert Pa to hPa for Blynk
                    lcd.setCursor(8, 1);
                    lcd.print("P:");
                    lcd.print(P / 100.0); // Display pressure in hPa
                } else {
                    Serial.println("Error reading pressure.");
                }
            } else {
                Serial.println("Error starting pressure measurement.");
            }
        } else {
            Serial.println("Error reading temperature.");
        }
    } else {
        Serial.println("Error starting temperature measurement.");
    }
}

void loop() {
    DHT11sensor();
    rainSensor();
    pressure();
    Blynk.run(); // Run the Blynk library
}
