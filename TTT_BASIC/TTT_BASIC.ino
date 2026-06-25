#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ====== WiFi & OpenAI Settings ======
const char* ssid = "home";
const char* password = "6strings";
String openai_api_key = "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";  // apni API key lagao

WebServer server(80);

// ====== Function: Get Response from OpenAI ======
String getOpenAIResponse(String question) {
  WiFiClientSecure client;
  client.setInsecure();  // SSL verification off

  HTTPClient https;
  String payload = "";

  if (https.begin(client, "https://api.openai.com/v1/chat/completions")) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", "Bearer " + openai_api_key);

    // ====== Request Body ======
    String requestBody = "{";
    requestBody += "\"model\": \"gpt-4.1-mini\",";
    requestBody += "\"max_tokens\": 50,";
    requestBody += "\"temperature\": 0,";
    requestBody += "\"messages\":[{\"role\":\"user\",\"content\":\"" + question + "\"}]";
    requestBody += "}";

    int httpResponseCode = https.POST(requestBody);

    if (httpResponseCode == 200) {
      payload = https.getString();
      Serial.println("API Response: " + payload);

      // ====== Parse JSON ======
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, payload);
      String answer = doc["choices"][0]["message"]["content"].as<String>();
      return answer;
    } else {
      Serial.println("Error: " + String(httpResponseCode));
      return "Error contacting OpenAI API";
    }
    https.end();
  }
  return "Connection Failed";
}

// ====== Web Page (Main Form) ======
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Chat</title></head><body>";
  html += "<h2>🤖 Ask me anything</h2>";
  html += "<form action='/ask' method='GET'>";
  html += "<input type='text' name='q' style='width:300px;'>";
  html += "<input type='submit' value='Ask'>";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// ====== Web Page (Show Answer) ======
void handleAsk() {
  if (server.hasArg("q")) {
    String question = server.arg("q");
    String answer = getOpenAIResponse(question);

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Answer</title></head><body>";
    html += "<h3><b>Your Question:</b> " + question + "</h3>";
    html += "<h3><b>AI Answer:</b><br>" + answer + "</h3>";
    html += "<br><a href='/'>🔙 Ask another question</a>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  } else {
    server.send(400, "text/plain", "No question provided.");
  }
}

void setup() {
  Serial.begin(115200);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP Address: ");
  Serial.println(WiFi.localIP());

  // Web Routes
  server.on("/", handleRoot);
  server.on("/ask", handleAsk);

  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();
}
