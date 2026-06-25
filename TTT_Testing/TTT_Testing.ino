#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "home";
const char* password = "6strings";
String apiKey = "sk-proj-XXXXXXXXXXXX"; // Apni API key dalain

WebServer server(80);

void handleRoot() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 AI Assistant</title>
      <style>
        body {
          margin: 0;
          font-family: Arial, sans-serif;
          background: linear-gradient(135deg, #667eea, #764ba2);
          height: 100vh;
          display: flex;
          justify-content: center;
          align-items: center;
          color: #333;
        }
        .card {
          background: #fff;
          padding: 30px;
          border-radius: 20px;
          box-shadow: 0 10px 25px rgba(0,0,0,0.2);
          max-width: 500px;
          width: 100%;
          text-align: center;
        }
        h1 {
          font-size: 24px;
          margin-bottom: 20px;
          color: #444;
        }
        input[type="text"] {
          width: 80%;
          padding: 10px;
          border: 2px solid #667eea;
          border-radius: 10px;
          font-size: 16px;
          outline: none;
        }
        button {
          margin-top: 15px;
          padding: 10px 20px;
          border: none;
          background: #667eea;
          color: white;
          border-radius: 10px;
          font-size: 16px;
          cursor: pointer;
          transition: 0.3s;
        }
        button:hover {
          background: #5563c1;
        }
        #answer {
          margin-top: 20px;
          padding: 15px;
          background: #f4f4f9;
          border-left: 5px solid #667eea;
          border-radius: 10px;
          text-align: left;
          display: none;
          animation: fadeIn 0.6s ease-in-out;
          max-height: 200px;
          overflow-y: auto;
        }
        @keyframes fadeIn {
          from {opacity: 0; transform: translateY(10px);}
          to {opacity: 1; transform: translateY(0);}
        }
      </style>
      <script>
        async function askAI() {
          let q = document.getElementById('question').value;
          if(q.trim() === '') return;
          document.getElementById('answer').style.display = 'block';
          document.getElementById('answer').innerHTML = "Thinking...";
          let res = await fetch('/ask?q=' + encodeURIComponent(q));
          let text = await res.text();
          document.getElementById('answer').innerHTML = text;
        }
      </script>
    </head>
    <body>
      <div class="card">
        <h1>ESP32 AI Assistant</h1>
        <input type="text" id="question" placeholder="Type your question...">
        <br>
        <button onclick="askAI()">Ask</button>
        <div id="answer"></div>
      </div>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleAsk() {
  if (!server.hasArg("q")) {
    server.send(400, "text/plain", "No question received");
    return;
  }
  
  String question = server.arg("q");
  Serial.println("User Question: " + question);

  HTTPClient http;
  http.begin("https://api.openai.com/v1/responses");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + apiKey);

  // ✅ Correct Request Body with max_tokens
  String requestBody = "{";
  requestBody += "\"model\": \"gpt-4.1-mini\",";
  requestBody += "\"max_tokens\": 35,";
  requestBody += "\"temperature\": 0,";
  requestBody += "\"input\": \"" + question + "\"";
  requestBody += "}";

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      String answer = doc["output"][0]["content"][0]["text"].as<String>();
      server.send(200, "text/plain", answer);
      Serial.println("AI Answer: " + answer);
    } else {
      server.send(500, "text/plain", "Error parsing JSON");
      Serial.println("Parsing error!");
    }
  } else {
    server.send(500, "text/plain", "Error connecting to API");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/ask", handleAsk);
  server.begin();
}

void loop() {
  server.handleClient();
}
