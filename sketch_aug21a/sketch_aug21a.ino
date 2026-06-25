#include <hal_conf_extra.h>
#include <stm32-config-i2s.h>
#include <stm32-i2s.h>

#include "Audio.h"

Audio::Audio(MicType micType) {
  wavData = new char*[wavDataSize/dividedWavDataSize];
  for (int i = 0; i < wavDataSize/dividedWavDataSize; ++i) wavData[i] = new char[dividedWavDataSize];
  i2s = new I2S(micType);
}

Audio::~Audio() {
  for (int i = 0; i < wavDataSize/dividedWavDataSize; ++i) delete[] wavData[i];
  delete[] wavData;
  delete i2s;
}

void Audio::CreateWavHeader(byte* header, int waveDataSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // linear PCM
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;
  header[22] = 0x01;  // monoral
  header[23] = 0x00;
  header[24] = 0x80;  // sampling rate 16000
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;  // Byte/sec = 16000x2x1 = 32000
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00;
  header[32] = 0x02;  // 16bit monoral
  header[33] = 0x00;
  header[34] = 0x10;  // 16bit
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(waveDataSize & 0xFF);
  header[41] = (byte)((waveDataSize >> 8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}

void Audio::Record() {
  CreateWavHeader(paddedHeader, wavDataSize);
  int bitBitPerSample = i2s->GetBitPerSample();
  if (bitBitPerSample == 16) {
    for (int j = 0; j < wavDataSize/dividedWavDataSize; ++j) {
      i2s->Read(i2sBuffer, i2sBufferSize/2);
      for (int i = 0; i < i2sBufferSize/8; ++i) {
        wavData[j][2*i] = i2sBuffer[4*i + 2];
        wavData[j][2*i + 1] = i2sBuffer[4*i + 3];
      }
    }
  }
  else if (bitBitPerSample == 32) {
    for (int j = 0; j < wavDataSize/dividedWavDataSize; ++j) {
      i2s->Read(i2sBuffer, i2sBufferSize);
      for (int i = 0; i < i2sBufferSize/8; ++i) {
        wavData[j][2*i] = i2sBuffer[8*i + 2];
        wavData[j][2*i + 1] = i2sBuffer[8*i + 3];
      }
    }
  }
}
#ifndef _AUDIO_H
#define _AUDIO_H

#include <Arduino.h>
#include "I2S.h"

// 16bit, monoral, 16000Hz,  linear PCM
class Audio {
  I2S* i2s;
  static const int headerSize = 44;
  static const int i2sBufferSize = 12000;
  char i2sBuffer[i2sBufferSize];
  void CreateWavHeader(byte* header, int waveDataSize);

public:
  static const int wavDataSize = 90000;                   // It must be multiple of dividedWavDataSize. Recording time is about 1.9 second.
  static const int dividedWavDataSize = i2sBufferSize/4;
  char** wavData;                                         // It's divided. Because large continuous memory area can't be allocated in esp32.
  byte paddedHeader[headerSize + 4] = {0};                // The size must be multiple of 3 for Base64 encoding. Additional byte size must be even because wave data is 16bit.

  Audio(MicType micType);
  ~Audio();
  void Record();
};

#endif // _AUDIO_H
#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
#define USE_SERIAL Serial
#include <Arduino.h>
#include <HTTPClient.h>
//#define uart_en 15
 #define led_3 4
 #define led_1 15
#define led_2 2
//#include <SoftwareSerial.h>
////SoftwareSerial (D4, D2);
const char* chatgpt_token = "Your_ChatGPT_Token";
CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
  this->authentication = authentication;
  WiFi.begin(ssid, password);
 //  while (WiFi.status() == WL_CONNECTED){ digitalWrite(led_3,1);}
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  client.setCACert(root_ca);


  if (!client.connect(server, 443)) Serial.println("Connection failed!"); digitalWrite(led_3,1);digitalWrite(led_1,0);digitalWrite(led_2,0);
}

String ans;

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio)
{
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", "");  // delete last "\n"
  client.print(enc);      // HttpBody2
  char** wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
    enc = base64::encode((byte*)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", "");// delete last "\n"
    client.print(enc);    // HttpBody2
  }
}

void CloudSpeechClient::Transcribe(Audio* audio) {
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-IN\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3; // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;
  // if (authentication == USE_APIKEY)
  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey
               + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  //  else if (authentication == USE_ACCESSTOKEN)
  //    HttpHeader = String("POST /v1beta1/speech:syncrecognize HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nAuthorization: Bearer ")
  //   + AccessToken + String("\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  client.print(HttpHeader);
  client.print(HttpBody1);
  PrintHttpBody2(audio);
  client.print(HttpBody3);
  String My_Answer="";
  while (!client.available());

  while (client.available())
  {
    char temp = client.read();
    My_Answer = My_Answer + temp;
   // Serial.write(client.read());
  }

 // Serial.print("My Answer - ");Serial.println(My_Answer);
  int postion = My_Answer.indexOf('{');
 // Serial.println(postion);
  ans = My_Answer.substring(postion);
  Serial.print("Json daata--");
  //Serial.print(ans);

 DynamicJsonDocument doc(384);

//StaticJsonDocument<384> doc;

DeserializationError error = deserializeJson(doc, ans);

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return;
}

JsonObject results_0 = doc["results"][0];
//const char* 
const char* chatgpt_Q = results_0["alternatives"][0]["transcript"];
const char* a= "light on";
const char* b= "light off";
//String chatgpt_Q = a+ans+b;
//Serial.println(ans);
Serial.print(chatgpt_Q);Serial.println("-");
///////////////////////////////////////////////////////////
pinMode(LED_BUILTIN,OUTPUT);
if(strstr(chatgpt_Q, "light on")){
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("Light's On");
  digitalWrite(15, LOW);
delay(1);
Serial2.println("Turning Light on");
digitalWrite(led_1,1);
digitalWrite(led_3,0);
digitalWrite(led_2,1);
//digitalWrite(uart_en,HIGH);
  Serial.print("To ask again");
  }
if(strstr(chatgpt_Q, "light off")){
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("Light's Off");
  digitalWrite(15, LOW);
delay(1);
Serial2.println("Turning Light off");
digitalWrite(led_1,1);
digitalWrite(led_2,1);
//digitalWrite(uart_en,HIGH);
  Serial.print("To ask again");
  }  
 if(strstr(chatgpt_Q, "blink on")){
  HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
        http.begin("http://example.com/index.html"); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
  } 
 else if(strstr(chatgpt_Q, "light on")==0 && strstr(chatgpt_Q, "light off") == 0 && strstr(chatgpt_Q, "blink on") ==0 ){
  Serial.println("Asking Chat GPT");
  HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin("https://api.openai.com/v1/completions")) {  // HTTPS

      https.addHeader("Content-Type", "application/json"); 
      String token_key = String("Bearer ") + chatgpt_token;
      https.addHeader("Authorization", token_key);

      String payload = String("{\"model\": \"text-davinci-003\", \"prompt\": ") +"\""+ chatgpt_Q +"\"" + String(", \"temperature\": 0.2, \"max_tokens\": 40}"); //Instead of TEXT as Payload, can be JSON as Paylaod

      Serial.print("[HTTPS] GET...\n");

      // start connection and send HTTP header
      int httpCode = https.POST(payload);

      // httpCode will be negative on error      
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
       // Serial2.println(payload);
        //////////////////////////////////////////////////
        StaticJsonDocument<2000> doc2;

DeserializationError error = deserializeJson(doc2, payload);

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return;

}
JsonObject choices_0 = doc2["choices"][0];
const char* only_ans = choices_0["text"];
Serial.println("Only ans:-");Serial.print(only_ans);
Serial2.print(only_ans);
digitalWrite(led_1,1);
digitalWrite(led_2,1);
//digitalWrite(uart_en, LOW);
delay(1);

//digitalWrite(uart_en,HIGH);
        /////////////////////////////////////////////////////////
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
    else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

  Serial.print("To ask again");
  //delay(10000);

  } 




///////////////////////////////////////////////////////////
/*
   
  */
}
#ifndef _CLOUDSPEECHCLIENT_H
#define _CLOUDSPEECHCLIENT_H
#include <WiFiClientSecure.h>
#include "Audio.h"

enum Authentication {
  USE_ACCESSTOKEN,
  USE_APIKEY
};

class CloudSpeechClient {
  WiFiClientSecure client;
  void PrintHttpBody2(Audio* audio);
  Authentication authentication;

public:
  CloudSpeechClient(Authentication authentication);
  ~CloudSpeechClient();
  void Transcribe(Audio* audio);
};

#endif // _CLOUDSPEECHCLIENT_H
#define led_1 15
#define led_2 2
#define button 23 //IR Sensor

#define led_3 4

#define RXp2 16
#define TXp2 17
#include "Audio.h"
#include "CloudSpeechClient.h"
int i=0;
void setup() {
pinMode(button, INPUT);
pinMode(led_1,OUTPUT);
pinMode(led_2,OUTPUT);
 pinMode(led_3,OUTPUT);
  Serial.begin(115200);
 Serial2.begin(115200, SERIAL_8N1, RXp2,TXp2);
   Serial2.println("Intialising");
//  Serial.println(My_Data);
}

void loop() {

     digitalWrite(led_1, 0);
     digitalWrite(led_2, 0);
      digitalWrite(led_3, 0);

   if(i==0){
   Serial.println("Press button");
   i=1;
   }
 //  if(i==1){delay(1);}

   delay(500);
   if(digitalRead(button)==0){
     Serial2.println("\r\nPlease Ask!\r\n");
     digitalWrite(led_1, 1);
     digitalWrite(led_2, 0);
      digitalWrite(led_3, 0);
     delay(2100);
  Serial.println("\r\nRecord start!\r\n");
  //Serial2.println("\r\nRecord start!\r\n");
  Audio* audio = new Audio(ADMP441);
 //Audio* audio = new Audio(M5STACKFIRE);
  audio->Record();
  Serial.println("Recoding Complited Processing");
  digitalWrite(led_1,0);
  digitalWrite(led_3,0);
  digitalWrite(led_2,1);
  CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);
  cloudSpeechClient->Transcribe(audio);
  delete cloudSpeechClient;
  delete audio;
  i=0;
  }
  if(digitalRead(button)==1){
    delay(1);

    }
}
#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 26
#define PIN_I2S_LRC 22
#define PIN_I2S_DIN 34
#define PIN_I2S_DOUT 25

// This I2S specification : 
//  -   LRC high is channel 2 (right).
//  -   LRC signal transitions once each word.
//  -   DATA is valid on the CLOCK rising edge.
//  -   Data bits are MSB first.
//  -   DATA bits are left-aligned with respect to LRC edge.
//  -   DATA bits are right-shifted by one with respect to LRC edges.
I2S::I2S(MicType micType) {
  if (micType == M5GO || micType == M5STACKFIRE ) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 2,
      .dma_buf_len = 1024
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_6);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
    i2s_adc_enable(I2S_NUM_0);
  }
  else if (micType == ADMP441 || micType == ICS43434 ) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 16,
      .dma_buf_len = 60
    };
    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = PIN_I2S_BCLK;
    pin_config.ws_io_num = PIN_I2S_LRC;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = PIN_I2S_DIN;
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
  }
}

int I2S::Read(char* data, int numData) {
  return i2s_read_bytes(I2S_NUM_0, (char *)data, numData, portMAX_DELAY);
}

int I2S::GetBitPerSample() {
  return (int)BITS_PER_SAMPLE;
}
#ifndef _I2S_H
#define _I2S_H
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"

enum MicType {
  ADMP441,
  ICS43434,
  M5GO,
  M5STACKFIRE
};

class I2S {
  i2s_bits_per_sample_t BITS_PER_SAMPLE;
public:
  I2S(MicType micType);
  int Read(char* data, int numData);
  int GetBitPerSample();
};

#endif // _I2S_H
#ifndef _NETWORK_PARAM_H
#define _NETWORK_PARAM_H

const char *ssid = "SSID";
const char *password = "PASSWORD";
const char*  server = "speech.googleapis.com";

// To get the certificate for your region run:
// openssl s_client -showcerts -connect speech.googleapis.com:443
// Copy the certificate (all lines between and including ---BEGIN CERTIFICATE---
// and --END CERTIFICATE--) to root.cert and put here on the root_cert variable.
const char* root_ca= 
"-----BEGIN CERTIFICATE-----\n"
"MIIFljCCA36gAwIBAgINAgO8U1lrNMcY9QFQZjANBgkqhkiG9w0BAQsFADBHMQsw\n"
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjAwODEzMDAwMDQyWhcNMjcwOTMwMDAw\n"
"MDQyWjBGMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
"Y2VzIExMQzETMBEGA1UEAxMKR1RTIENBIDFDMzCCASIwDQYJKoZIhvcNAQEBBQAD\n"
"ggEPADCCAQoCggEBAPWI3+dijB43+DdCkH9sh9D7ZYIl/ejLa6T/belaI+KZ9hzp\n"
"kgOZE3wJCor6QtZeViSqejOEH9Hpabu5dOxXTGZok3c3VVP+ORBNtzS7XyV3NzsX\n"
"lOo85Z3VvMO0Q+sup0fvsEQRY9i0QYXdQTBIkxu/t/bgRQIh4JZCF8/ZK2VWNAcm\n"
"BA2o/X3KLu/qSHw3TT8An4Pf73WELnlXXPxXbhqW//yMmqaZviXZf5YsBvcRKgKA\n"
"gOtjGDxQSYflispfGStZloEAoPtR28p3CwvJlk/vcEnHXG0g/Zm0tOLKLnf9LdwL\n"
"tmsTDIwZKxeWmLnwi/agJ7u2441Rj72ux5uxiZ0CAwEAAaOCAYAwggF8MA4GA1Ud\n"
"DwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0T\n"
"AQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUinR/r4XN7pXNPZzQ4kYU83E1HScwHwYD\n"
"VR0jBBgwFoAU5K8rJnEaK0gnhS9SZizv8IkTcT4waAYIKwYBBQUHAQEEXDBaMCYG\n"
"CCsGAQUFBzABhhpodHRwOi8vb2NzcC5wa2kuZ29vZy9ndHNyMTAwBggrBgEFBQcw\n"
"AoYkaHR0cDovL3BraS5nb29nL3JlcG8vY2VydHMvZ3RzcjEuZGVyMDQGA1UdHwQt\n"
"MCswKaAnoCWGI2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzcjEvZ3RzcjEuY3JsMFcG\n"
"A1UdIARQME4wOAYKKwYBBAHWeQIFAzAqMCgGCCsGAQUFBwIBFhxodHRwczovL3Br\n"
"aS5nb29nL3JlcG9zaXRvcnkvMAgGBmeBDAECATAIBgZngQwBAgIwDQYJKoZIhvcN\n"
"AQELBQADggIBAIl9rCBcDDy+mqhXlRu0rvqrpXJxtDaV/d9AEQNMwkYUuxQkq/BQ\n"
"cSLbrcRuf8/xam/IgxvYzolfh2yHuKkMo5uhYpSTld9brmYZCwKWnvy15xBpPnrL\n"
"RklfRuFBsdeYTWU0AIAaP0+fbH9JAIFTQaSSIYKCGvGjRFsqUBITTcFTNvNCCK9U\n"
"+o53UxtkOCcXCb1YyRt8OS1b887U7ZfbFAO/CVMkH8IMBHmYJvJh8VNS/UKMG2Yr\n"
"PxWhu//2m+OBmgEGcYk1KCTd4b3rGS3hSMs9WYNRtHTGnXzGsYZbr8w0xNPM1IER\n"
"lQCh9BIiAfq0g3GvjLeMcySsN1PCAJA/Ef5c7TaUEDu9Ka7ixzpiO2xj2YC/WXGs\n"
"Yye5TBeg2vZzFb8q3o/zpWwygTMD0IZRcZk0upONXbVRWPeyk+gB9lm+cZv9TSjO\n"
"z23HFtz30dZGm6fKa+l3D/2gthsjgx0QGtkJAITgRNOidSOzNIb2ILCkXhAd4FJG\n"
"AJ2xDx8hcFH1mt0G/FX0Kw4zd8NLQsLxdxP8c4CU6x+7Nz/OAipmsHMdMqUybDKw\n"
"juDEI/9bfU1lcKwrmz3O2+BtjjKAvpafkmO8l7tdufThcV4q5O8DIrGKZTqPwJNl\n"
"1IXNDw9bg1kWRxYtnCQ6yICmJhSFm/Y3m6xv+cXDBlHz4n/FsRC6UfTd\n"
"-----END CERTIFICATE-----\n";

// Getting Access Token : 
// At first, you should get service account key (JSON file).
// Type below command in Google Cloud Shell to get AccessToken: 
// $ gcloud auth activate-service-account --key-file=KEY_FILE   (KEY_FILE is your service account key file)
// $ gcloud auth print-access-token
// The Access Token is expired in an hour.
// Google recommends to use Access Token.
//const String AccessToken = "";

// It is also possible to use "API Key" instead of "Access Token". It doesn't have time limit.
const String ApiKey = "Your_API_Key";

// see https://cloud.google.com/docs/authentication?hl=ja#getting_credentials_for_server-centric_flow
// see https://qiita.com/basi/items/3623a576b754f738138e (Japanese)

#endif  // _NETWORK_PARAM_H
 #include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#define uart_en 15
#define RXp2 16
#define TXp2 17
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;


void setup()
{

  Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXp2,TXp2);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin( "SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED)
    delay(1500);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
  audio.connecttospeech("Starting ", "en"); // Google TTS
}


void loop()

{
if (Serial2.available()){
  String Answer = Serial2.readString();
  Serial.println(Answer);
 audio.connecttospeech(Answer.c_str(), "en");
  }
  audio.loop();
}

void audio_info(const char *info) {
  Serial.print("audio_info: "); Serial.println(info);}