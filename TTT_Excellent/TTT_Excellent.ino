#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ==== Replace with your WiFi and API key ====
const char* ssid = "home";
const char* password = "6strings";
String apiKey = "Your API Key";   // OpenAI API key
// ===========================================

WebServer server(80);

String sendQuestionToOpenAI(String question) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String endpoint = "https://api.openai.com/v1/responses";
  https.begin(client, endpoint);

  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", "Bearer " + apiKey);

    // ====== Request Body ======
    String requestBody = "{";
    requestBody += "\"model\": \"gpt-4.1-mini\",";
    requestBody += "\"max_tokens\": 35,";
    requestBody += "\"temperature\": 0,";
    requestBody += "\"messages\":[{\"role\":\"user\",\"content\":\"" + question + "\"}]";
    requestBody += "}";
    
  String payload = "{\"model\": \"gpt-4.1-mini\", \"input\": \"" + question + "\"}";

  int httpCode = https.POST(payload);

  String response = "";
  if (httpCode > 0) {
    response = https.getString();
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      response = doc["output"][0]["content"][0]["text"].as<String>();
    } else {
      response = "Error parsing response";
    }
  } else {
    response = "HTTP Request failed";
  }

  https.end();
  return response;
}

// ==== Stylish Robotics Style Webpage ====
String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>🤖 Robo-Assistant</title>
  <style>
    body {
      margin: 0; padding: 0;
      font-family: 'Orbitron', sans-serif;
      background: radial-gradient(circle at center, #0f2027, #203a43, #2c5364);
      color: #00fff7;
      text-align: center;
    }
    h1 {
      margin-top: 30px;
      font-size: 2.5em;
      color: #00eaff;
      text-shadow: 0px 0px 15px #00f7ff;
    }
    .container {
      margin: 40px auto;
      width: 80%;
      max-width: 600px;
      background: rgba(0,0,0,0.6);
      border: 2px solid #00f7ff;
      border-radius: 20px;
      padding: 20px;
      box-shadow: 0px 0px 20px #00f7ff;
    }
    input[type=text] {
      width: 80%;
      padding: 12px;
      font-size: 1em;
      border-radius: 10px;
      border: 2px solid #00fff7;
      background: black;
      color: #00fff7;
      outline: none;
    }
    input[type=submit] {
      padding: 12px 25px;
      margin-top: 20px;
      font-size: 1em;
      font-weight: bold;
      border: none;
      border-radius: 10px;
      background: #00f7ff;
      color: black;
      cursor: pointer;
      transition: 0.3s;
    }
    input[type=submit]:hover {
      background: #ff00d4;
      color: white;
      box-shadow: 0px 0px 15px #ff00d4;
    }
    .answer-box {
      margin-top: 30px;
      padding: 20px;
      border-radius: 10px;
      background: #001f29;
      border: 2px solid #00fff7;
      color: #00fff7;
      font-size: 1.2em;
      box-shadow: 0px 0px 15px #00f7ff inset;
    }
  </style>
  <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@500&display=swap" rel="stylesheet">
</head>
<body>
  <h1>🤖 Robo Assistant</h1>
  <div class="container">
    <form action="/" method="get">
      <input type="text" name="q" placeholder="Type your question...">
      <br>
      <input type="submit" value="Ask">
    </form>
    %ANSWER%
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  String q = server.hasArg("q") ? server.arg("q") : "";
  String page = htmlPage;
  if (q != "") {
    String answer = sendQuestionToOpenAI(q);
    String answerBox = "<div class='answer-box'><b>Answer:</b><br>" + answer + "</div>";
    page.replace("%ANSWER%", answerBox);
  } else {
    page.replace("%ANSWER%", "");
  }
  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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
