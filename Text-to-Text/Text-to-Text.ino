#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "home";
const char* password = "6strings";

// OpenAI API
const char* api_host = "api.openai.com";
String api_key = "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";   // apni API key yahan likho
String model = "gpt-4.1-mini";

WebServer server(80);

void handleRoot() {
  if (server.hasArg("q")) {
    String question = server.arg("q");
    Serial.println("Your Question: " + question);

    // API call
    WiFiClientSecure client;
    client.setInsecure();  // SSL certificate ignore karne ke liye

    if (!client.connect(api_host, 443)) {
      server.send(500, "text/plain", "Connection to API failed");
      return;
    }

    // JSON body
    String body = "{ \"model\": \"" + model + "\", \"messages\": [ {\"role\": \"user\", \"content\": \"" + question + "\" } ] }";

    // HTTP request
    String request = String("POST /v1/chat/completions HTTP/1.1\r\n") +
                     "Host: " + api_host + "\r\n" +
                     "Authorization: Bearer " + api_key + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + body.length() + "\r\n" +
                     "Connection: close\r\n\r\n" +
                     body;

    client.print(request);

    // Response read
    String response = "";
    while (client.connected() || client.available()) {
      String line = client.readStringUntil('\n');
      response += line + "\n";
    }

    // Extract JSON part
    int jsonIndex = response.indexOf("{");
    if (jsonIndex == -1) {
      server.send(500, "text/plain", "Invalid API response");
      return;
    }

    String jsonResponse = response.substring(jsonIndex);
    Serial.println("Raw Response:\n" + jsonResponse);

    // Parse JSON
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error) {
      server.send(500, "text/plain", "JSON Parse failed");
      return;
    }

    // Extract answer
    const char* answer = doc["choices"][0]["message"]["content"];
    Serial.println("AI Answer: " + String(answer));

    // Send to browser
    server.send(200, "text/plain", "Q: " + question + "\nA: " + String(answer));
  } else {
    server.send(200, "text/plain", "Send a question using /?q=your+question");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}