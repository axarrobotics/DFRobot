#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "sample.h"   // generated with xxd -i
#include "wav_header.h"
#include "camera.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ESP_I2S.h"
#include "cJSON.h"
#include <base64.h>
#include <WebServer.h>
#include "Preferences.h"
#define BUTTON_PIN 0
#define LED_PIN 3
#define SAMPLE_RATE     (4000) // The sample rate is tested with OpenAI and selected. 
#define DATA_PIN        (GPIO_NUM_39)
#define CLOCK_PIN       (GPIO_NUM_38)

// int counter = 0;
WebServer server(80);
base64 base64_conv;
//Other IMP MACROS
#define SERVER_RESP_TIMEOUT_MS 40000 
//For I2S_audio
I2SClass I2S;
bool isRecording = false; 
uint8_t *wavBuffer = NULL;
size_t wavBufferSize = 0; 

// Replace with your credentials
const char* sta_ssid = "Test";
const char* sta_password = "Test123@";
const char* ap_ssid = "ESP_ROBOT";
const char* ap_password = "Robot123@";
String openaiKey = "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";
int num_of_audio_iterations = 0;
int remaining_audio_len = 0;
// int num_of_image_iterations = 0;
// int remaining_image_len = 0;
String result;
String audio_transcripted_txt;
String image_answer_txt;

//For Recording start and stop
#define START_RECORDING 0
#define STOP_RECORDING 1
int recording_state = STOP_RECORDING;

//For Mode Changes
Preferences preferences;
unsigned int mode;
#define OPEN_AI_QA 1
#define ESP_ROLL 0 


  // uint8_t buffer[50000]; 

// #ifdef test
// ISRG Root X1 certificate (valid for api.openai.com)
// const char* root_ca = \
// "-----BEGIN CERTIFICATE-----\n"
// "MIIFazCCA1OgAwIBAgISA6UjtpRwaP1pY9lKGVt7lU5RMA0GCSqGSIb3DQEBCwUA\n"
// "MEoxCzAJBgNVBAYTAlVTMRMwEQYDVQQKDApMZXQncyBFbmNyeXB0MSMwIQYDVQQD\n"
// "DBpMZXQncyBFbmNyeXB0IFJvb3QgQ0EgWDEwHhcNMjAwNDE1MDAwMDAwWhcNMjMw\n"
// "NTExMTIwMDAwWjA6MQswCQYDVQQGEwJVUzETMBEGA1UECgwKTGV0J3MgRW5jcnlw\n"
// "dDEjMCEGA1UEAwwaTGV0J3MgRW5jcnlwdCBSb290IENBIFgxMIIBIjANBgkqhkiG\n"
// "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzNPdF2dAhtqgWpyfcU5q5o8rZg1aI5WJxks/\n"
// "3u7QzTz1U5WZty4LYtL3zM3vG+6ZChqPOeTyy0MMHQhdGjw6a5zXgtrZD2jE3j9t\n"
// "H+Q6h/4gkXWm+dc6ZgIGpTuH/uhV7KaKnWXexsb6LSSUnk1wUFV0UodWIaUgVRa3\n"
// "nsi2MJl2UuQMSiYq8rrfNIThO3WAnV4Tp5z5vI3oLShM2ZkH01CM4Dk5W7vY6D0T\n"
// "mkr1+8PIFzH5o2h3iFz6yC0+9F7GeyXKaqcrX8iFMSmI5PgZsQCKA+J4sF+PP7oM\n"
// "6e3hUjBfGfRKnQ60r1lWB3yZlIUV73pR/3nGMyGH0z3gH4S4RwIDAQABo4ICZzCC\n"
// "AmMwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcD\n"
// "AjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBSoSmpjBH3DU7g79IYz1n34RBduvjAf\n"
// "BgNVHSMEGDAWgBQULrMXt1hWy65QCUDmH6+dixTCxjA4BggrBgEFBQcBAQIwMC4G\n"
// "CCsGAQUFBzABhiJodHRwOi8vb2NzcC5pbnQtbGV0c2VuY3J5cHQub3JnMIIBAwYI\n"
// "KwYBBQUHAQsEgfowgfcwKwYIKwYBBQUHMAGGH2h0dHA6Ly9vY3NwLmludC1sZXRz\n"
// "ZW5jcnlwdC5vcmcwNQYIKwYBBQUHMAKGKWh0dHA6Ly9jZXJ0LmNhY2VydC5vcmcv\n"
// "bGV0c2VuY3J5cHQucm9vdHgubmV3LmNydDA/BggrBgEFBQcwAoYzaHR0cDovL2Nh\n"
// "Y2VydC5pbnQtbGV0c2VuY3J5cHQub3JnL2xldHNlbmNyeXB0LXJvb3QueDEuY2Vy\n"
// "MEUGA1UdIAQ+MDwwOgYEVR0gADAyMDAGCCsGAQUFBwIBFiRodHRwczovL2xldHNl\n"
// "bmNyeXB0Lm9yZy9yZXBvc2l0b3J5LzA8BgNVHR8ENTAzMDGgL6AthitodHRwOi8v\n"
// "Y3JsLmludC1sZXRzZW5jcnlwdC5vcmcvbGV0c2VuY3J5cHQtcjEuY3JsMA0GCSqG\n"
// "SIb3DQEBCwUAA4IBAQAfjYONglz8q6SyQHhX+3vSeKywvwwUMN5D0ZDKkuEv/Lnl\n"
// "AJZZHYhHBGQFHoJ0L3hPiS14WraUQchSUg36Xx2DJBgPKZW/zRLvcmjO2VXfDgeU\n"
// "90PY9l00ZVY0Dw+dYr4fZb+mT0hxQY11rIHTU6IY6PkUvPoUVeKq8YGHeA8lHk5Q\n"
// "ZGc7ISZdkV73Xx/KuD3W9tA1XkxTY6EfdM4DNXVutYPzmzMkmJ8th6Ep+bI7AN0V\n"
// "2XtsYUrpMhuJB+HtjCgLe3H8/3KwVjFuS7Qn2V/gP0m3C5ZPQ66n8B2CNkeJq8Jp\n"
// "TAxJ/dfAyNqFu8lfFWsByLbnL9XVrWYaM3CeYs+m\n"
// "-----END CERTIFICATE-----\n";
// #endif


void setup() {
  Serial.begin(115200);

  //Load variable from NVS and check the firmware to run
  preferences.begin("my-app", false);
  mode = preferences.getUInt("mode", 0);
    preferences.end();

  if(mode == OPEN_AI_QA)
  {
     Serial.println("OPEN_AI_QA firmware running");
  }
  else
  {
    Serial.println("ESP_ROLL Robot firmware running");
  }

    pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  audioInit();
  initCamera();
  WiFi.mode(WIFI_MODE_APSTA); //Set the device to operate in AP and STA modes simultaneously. 
  WiFi.begin(sta_ssid, sta_password); //Begin the STA mode and establish connection. 
  WiFi.softAP(ap_ssid, ap_password);  // Start the access point

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  //IP address of ESP32_AP 
    IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  //Set server configs and initiate it.
    server.on("/audio_record", handle_ForRecording); //For audio_record page
    server.on("/mode_change", handle_ModeChange); //For mode_change page
  server.on("/record", handleRecord); // Handle record_button toggle
  server.on("/toggle", handleMode);
  // server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  // imageAnswering("Describe this image for me");
  //  result = openAI_transcribe((uint8_t *)sample_wav, sample_wav_len);
  // Serial.println("=== OpenAI Response ===");
  // Serial.println(result);
}

void loop() {
    server.handleClient();

    if (START_RECORDING == recording_state) {
        if (!isRecording) {
          digitalWrite(LED_PIN, HIGH);
            startRecording();
        }
    } else {
        if (isRecording) {
            digitalWrite(LED_PIN, LOW);
            stopRecording();
        }
    }
    if (isRecording) {
        size_t bytesAvailable = I2S.available();
        if (bytesAvailable > 0) {
            uint8_t *newBuffer = (uint8_t *)realloc(wavBuffer, wavBufferSize + bytesAvailable);
            if (newBuffer == NULL) {
                log_e("Failed to reallocate WAV buffer");
                stopRecording();
                return;
            }
            wavBuffer = newBuffer;

            size_t bytesRead = I2S.readBytes((char *)(wavBuffer + wavBufferSize), bytesAvailable);
            wavBufferSize += bytesRead;
        }
    }

    delay(10); 
    }

    void startRecording() {
    size_t sampleRate = I2S.rxSampleRate();
    uint16_t sampleWidth = (uint16_t)I2S.rxDataWidth();
    uint16_t numChannels = (uint16_t)I2S.rxSlotMode();

    wavBufferSize = 0;
    wavBuffer = (uint8_t *)malloc(PCM_WAV_HEADER_SIZE);
    if (wavBuffer == NULL) {
        log_e("Failed to allocate WAV buffer");
        return;
    }

    const pcm_wav_header_t wavHeader = PCM_WAV_HEADER_DEFAULT(0, sampleWidth, sampleRate, numChannels);
    memcpy(wavBuffer, &wavHeader, PCM_WAV_HEADER_SIZE);
    wavBufferSize = PCM_WAV_HEADER_SIZE;

    Serial.println("Start recording...");
    isRecording = true;
}

//Handle RecordButtonToggle
void handleRecord() {
  if (server.hasArg("state")) {
    String state = server.arg("state");  // read query parameter
    Serial.println("Recording state: " + state);

    if (state == "start") {
      // do something when recording starts
      recording_state = START_RECORDING;
    } else if (state == "stop") {
      // do something when recording stops
      recording_state = STOP_RECORDING;
    }
  }
  server.send(200, "text/plain", "OK");
}

//Handle ModeButtonToggle
void handleMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");  // read query parameter
    Serial.println("Toggle state: " + mode);
    int value = 0;
    if(mode == "OPEN_AI_QA")
    {
      Serial.println("OPEN_AI_QA firmware set");
      value = OPEN_AI_QA;
    }
    else
    {
      Serial.println("ESP_ROLL firmware set");
      value = ESP_ROLL;
    }
     
     //Set the value to NVS
    preferences.begin("my-app", false);
  preferences.putUInt("mode", value);
    preferences.end();
  server.send(200, "text/plain", "Restarting Device");
  delay(3000);
  esp_restart();
}
}

void handle_ForRecording() {
  // counter++;
  server.send(200, "text/html", recordingPageHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void handle_ModeChange() {
  server.send(200, "text/html", modeChangeHTML());
}

String recordingPageHTML() {
  String str = "<!DOCTYPE html><html>";
  str += "<head><meta charset=\"UTF-8\"><title>Recording Control</title>";
  str += "<style>";
  str += "body {font-family: Arial, sans-serif; text-align: center; padding-top: 50px;}";
  str += "#recordBtn {background-color: #e74c3c; color: white; border: none; padding: 15px 30px;";
  str += "font-size: 18px; border-radius: 8px; cursor: pointer; transition: background-color 0.3s;}";
  str += "#recordBtn:hover {background-color: #c0392b;}";
  str += "</style>";
  str += "</head>";
  str += "<body>";
  str += "<h2>Recording Control</h2>";
  str += "<button id=\"recordBtn\" onclick=\"toggleRecording()\">RECORDING_START</button>";
  str += "<script>";
  str += "let recording = false;";
  str += "function toggleRecording() {";
  str += "  const btn = document.getElementById('recordBtn');";
  str += "  recording = !recording;";
  str += "  if (recording) {";
  str += "    btn.innerText = 'RECORDING_STOP';";
  str += "    sendUpdate('start');";
  str += "  } else {";
  str += "    btn.innerText = 'RECORDING_START';";
  str += "    sendUpdate('stop');";
  str += "  }";
  str += "}";
  str += "function sendUpdate(state) {";
  str += "  const xhr = new XMLHttpRequest();";
  str += "  xhr.open('GET', '/record?state=' + state, true);";
  str += "  xhr.send();";
  str += "}";
  str += "</script>";
  str += "</body></html>";
  return str;
}

String modeChangeHTML() {
  String str = "<!DOCTYPE html><html>";
  str += "<head><meta charset=\"UTF-8\"><title>ESP32 Mode Toggle</title>";
  str += "<style>";
  str += "body {font-family: Arial, sans-serif; text-align: center; padding-top: 50px;}";
  str += ".switch {position: relative; display: inline-block; width: 60px; height: 34px;}";
  str += ".switch input {opacity: 0; width: 0; height: 0;}";
  str += ".slider {position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;";
  str += "background-color: #ccc; transition: .4s; border-radius: 34px;}";
  str += ".slider:before {position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px;";
  str += "background-color: white; transition: .4s; border-radius: 50%;}";
  str += "input:checked + .slider {background-color: #4CAF50;}";
  str += "input:checked + .slider:before {transform: translateX(26px);}";
  str += "</style>";
  str += "</head>";
  str += "<body>";
  str += "<h2>ESP32 Mode Toggle</h2>";
  str += "<label class=\"switch\">";

  if(mode == OPEN_AI_QA)
  str += "<input type=\"checkbox\" id=\"modeSwitch\" onchange=\"toggleMode()\"checked>";
  else
  str += "<input type=\"checkbox\" id=\"modeSwitch\" onchange=\"toggleMode()\">";

  str += "<span class=\"slider\"></span>";
  str += "</label>";
  if(mode == OPEN_AI_QA)
  str += "<p id=\"status\">Mode: OPEN_AI_QA</p>";
  else
  str += "<p id=\"status\">Mode: ESP_ROLL</p>";
 
  str += "<script>";
  str += "function toggleMode() {";
  str += " var checkBox = document.getElementById('modeSwitch');";
  str += " var statusText = document.getElementById('status');";
  str += " var mode = checkBox.checked ? 'OPEN_AI_QA' : 'ESP_ROLL';";
  str += " statusText.innerHTML = 'Mode: ' + mode;";
  str += " var xhr = new XMLHttpRequest();";
  str += " xhr.open('GET', '/toggle?mode=' + mode, true);";
  str += " xhr.send();";
  str += "}";
  str += "</script>";
  str += "</body></html>";
  return str;
}


void audioInit()
{
  I2S.setPinsPdmRx(CLOCK_PIN, DATA_PIN);
  if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("Failed to initialize I2S PDM RX");
  }
}


String parse_response_for_audio(String txt)
{
    const char* resp = txt.c_str();
   char *resp1 = strstr(resp, "\"text\":");

   for(int i = 8; i <= 100; i ++)
   {
    if(*(resp1 + i) != '"')
    {
      audio_transcripted_txt += String(*(resp1 + i));
    }
    else
    {
      break;
    }
   }

   return audio_transcripted_txt;

}

String parse_response_for_image(String txt)
{
      const char* resp = txt.c_str();
   char *resp1 = strstr(resp, "\"content\":");

   for(int i = 12; i <= 500; i ++)
   {
    if(*(resp1 + i) != '"')
    {
      image_answer_txt += String(*(resp1 + i));
    }
    else
    {
      break;
    }
   }

   return image_answer_txt;

}

void stopRecording() {
    if (!isRecording) return;

    isRecording = false;
    
    Serial.printf("Stop recording. Total recorded size: %u\n", wavBufferSize);

    pcm_wav_header_t *header = (pcm_wav_header_t *)wavBuffer;
    header->descriptor_chunk.chunk_size =  (wavBufferSize) + sizeof(pcm_wav_header_t) - 8;
    header->data_chunk.subchunk_size = wavBufferSize - PCM_WAV_HEADER_SIZE;

    Serial.println("Start speech to text! ");
 
  String txt = openAI_transcribe(wavBuffer, wavBufferSize);
// Serial.println("312");
// Serial.print(txt);
    free(wavBuffer);
    if(txt!="Connection failed" && txt!=""){
        audio_transcripted_txt = parse_response_for_audio(txt);
        Serial.printf("SpeechToText Message: %s\n", audio_transcripted_txt.c_str());
        Serial.println("Start image Answering! ");
        txt=imageAnswering(audio_transcripted_txt);
        audio_transcripted_txt = "";
        if(txt!=""){
          // Serial.printf("imageAnswering Message:\n%s\n", txt.c_str());
          image_answer_txt = parse_response_for_image(txt);
          Serial.print("imageAnswering Message: ");
          Serial.println(image_answer_txt);
          Serial.println("Start text to audio! ");
          if(TextToSpeech(image_answer_txt)==-1){
            Serial.println("Audio reception failed! ");
          }
          image_answer_txt = "";
        }else{
          Serial.println("imageAnswering failed!");
        }
    }else{
      Serial.println("speech to text failed!");
    }
}

int TextToSpeech(String input_text)
{
  //Create JSON Object
    cJSON *message = cJSON_CreateObject();
  if (!message) { 
    Serial.println("JSON Creation failed");
    return 0;
  }
    cJSON_AddStringToObject(message, "model", "tts-1");
  cJSON_AddStringToObject(message, "voice", "alloy");
  cJSON_AddStringToObject(message, "input", input_text.c_str());
  cJSON_AddStringToObject(message, "response_format", "wav");
  String jsonBody = String(cJSON_Print(message));


  I2SClass i2s;
  i2s.setPins(45, 46, 42);
  if (!i2s.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("MAX98357 initialization failed!");
  }

  HTTPClient http;
  http.setTimeout(60000);
  http.useHTTP10(true);
  http.begin("https://api.openai.com/v1/" + String("audio/speech"));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + openaiKey);

  int httpCode = http.POST(jsonBody);

  int totalBytesRead = -1; 
   if (httpCode != HTTP_CODE_OK) {
      log_e("HTTP_ERROR: %d", httpCode);
  } else {
      WiFiClient* stream = http.getStreamPtr();
      int tempTime = 0;

      // memset(buffer, 0, sizeof(buffer));
      while (http.connected()) {
          size_t tempSize = stream->available();
          static int i=0;
          uint8_t buffer[1024];
          if (tempSize) {
              tempTime = 0; 
              size_t bytesToRead = std::min(tempSize, sizeof(buffer)); 
              size_t bytesRead = stream->read(buffer, bytesToRead);
              if(bytesRead>0){
                if(i==0){
                  i++;
                  pcm_wav_header_t *header = (pcm_wav_header_t *)buffer;
                  if (header->fmt_chunk.audio_format != 1) {
                    log_e("Audio format is not PCM!");
                    http.end();
                    return -1;
                  }
                  i2s.configureTX(header->fmt_chunk.sample_rate, (i2s_data_bit_width_t)header->fmt_chunk.bits_per_sample, (i2s_slot_mode_t)header->fmt_chunk.num_of_channels);
                  i2s.write(buffer+44, bytesRead-44);
                }else{
                  i2s.write(buffer,bytesRead);
                }
              }
              totalBytesRead += bytesRead; // 更新已读取的字节数
          } else {
              tempTime++;
              if (tempTime > 2500) { // 超时处理 
                  break;
              }
          }
          delay(1);
      }
  }

  http.end();
  Serial.print("Text to audio conversion done. Total bytes read : ");
  Serial.println(totalBytesRead);
  return totalBytesRead;
}

String openAI_transcribe(uint8_t *audio_data, uint32_t audio_len) {
  WiFiClientSecure client;

  //Compute the variables to send audio data
  if(audio_len >= 15500)
  {
    num_of_audio_iterations = (audio_len / 15500);
    if((audio_len % 15500) != 0)
    {
      remaining_audio_len = (audio_len % 15500);
    }
  }
  else
  {
    num_of_audio_iterations = 0;
    remaining_audio_len = 0;
  }


  // Use insecure for testing (replace later with proper root CA)
  
  client.setInsecure();
  //  client.setCACert(root_ca);  // ✅ use real certificate

  Serial.println("Connecting to api.openai.com ...");
  if (!client.connect("api.openai.com", 443)) {
    return "Connection failed";
  }
  Serial.println("Connected!");

  String boundary = "----ESP32Boundary";
  String bodyStart =
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"model\"\r\n\r\n"
    "gpt-4o-mini-transcribe\r\n"
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"file\"; filename=\"sample.wav\"\r\n"
    "Content-Type: audio/wav\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";
  int contentLength = bodyStart.length() + audio_len + bodyEnd.length();

  // ---- HTTP request ----
  client.println("POST /v1/audio/transcriptions HTTP/1.1");
  client.println("Host: api.openai.com");
  client.println("Authorization: Bearer " + openaiKey);
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(contentLength));
  client.println("Connection: close");
  client.println();

  // ---- Write body ----
  client.print(bodyStart);
  if(num_of_audio_iterations > 0)
  {
    for(int i = 0; i < num_of_audio_iterations; i ++)
    {
      client.write((audio_data + (i * 15500)), 15500);
      delay(100);
    }

    if(remaining_audio_len > 0)
    {
      client.write((audio_data + (num_of_audio_iterations * 15500)), remaining_audio_len);
    }

  }
  else
  {
  client.write(audio_data, audio_len);  // binary audio
  }
  client.print(bodyEnd);

  // ---- Read response ----
  String response = "";
  unsigned long timeout = millis();
  while (client.connected() || client.available()) {
    if (client.available()) {
      response += client.readStringUntil('\n');
      timeout = millis(); // reset watchdog
    }
    if (millis() - timeout > SERVER_RESP_TIMEOUT_MS) {  // 10s timeout
      break;
    }
  }
  Serial.println("exiting audio transcription");

  client.stop();
  // if(response == NULL)
  // {
  //   return "";
  // }
  return response;
}

String imageAnswering(String txt) {
  camera_fb_t *fb = NULL; 
  String response;

  // Capture picture
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    return "Camera capture failed";
  }

  Serial.println("Capturing image...");
  delay(5);

  Serial.println("Sending image to OpenAI");
  Serial.print("Length of the image data: ");
  Serial.println(fb->len); 

  response = transmit_chat_and_image(txt, fb->buf, fb->len);

  // Release frame buffer AFTER you're done with it
  esp_camera_fb_return(fb);

  return response;
}

/*
text with Image stucture:
    {
      "role": "user",
      "content": [
        {
          "type": "text",
          "text": "content"
        },
        {
          "type": "image_url",
          "image_url": {
            "url": "data:image/jpeg;base64,{image}"
          }
        }
      ]
    }
*/
String transmit_chat_and_image(String txt, uint8_t *image_data, long unsigned int imageLength)
{
  // Calculate Base64 length
    String image_data_str = base64_conv.encode(image_data, imageLength);

    WiFiClientSecure client;
    String endpoint = "chat/completions";

  // Create root JSON object
  cJSON *root = cJSON_CreateObject();
  // Add model
  cJSON_AddStringToObject(root, "model", "gpt-4o-mini");

  // Create messages array
  cJSON *messages = cJSON_CreateArray();
  cJSON_AddItemToObject(root, "messages", messages);

  // First message object
  cJSON *message = cJSON_CreateObject();
  cJSON_AddItemToArray(messages, message);

  cJSON_AddStringToObject(message, "role", "user");

  // content array
  cJSON *contentArray = cJSON_CreateArray();
  cJSON_AddItemToObject(message, "content", contentArray);

  // Text content
  cJSON *textContent = cJSON_CreateObject();
  cJSON_AddStringToObject(textContent, "type", "text");
  cJSON_AddStringToObject(textContent, "text", txt.c_str());
  cJSON_AddItemToArray(contentArray, textContent);

  // Image content
  cJSON *imageContent = cJSON_CreateObject();
  cJSON_AddStringToObject(imageContent, "type", "image_url");

  // Nested image_url object
  cJSON *imageUrlObj = cJSON_CreateObject();

  String urlString = "data:image/png;base64," + image_data_str;
  cJSON_AddStringToObject(imageUrlObj, "url", urlString.c_str());

  cJSON_AddItemToObject(imageContent, "image_url", imageUrlObj);
  cJSON_AddItemToArray(contentArray, imageContent);
  // Print JSON
  char *jsonString = cJSON_Print(root);

  // free(jsonString);
  // Serial.println("Request JSON:");
  // Serial.println(jsonString);
  // Serial.print("JSON Size : ");
  // Serial.println(strlen(jsonString));
  // int image_string_len = strlen(jsonString);
    //Compute the variables to send image data
  // if(image_string_len >= 2000)
  // {
  //   num_of_image_iterations = (image_string_len / 2000);
  //   if((image_string_len % 2000) != 0)
  //   {
  //     remaining_image_len = (image_string_len % 2000);
  //   }
  // }
  // else
  // {
  //   num_of_image_iterations = 0;
  //   remaining_image_len = 0;
  // }
   
  //  #ifdef test
  //   // Use insecure for testing (replace later with proper root CA)
  // client.setInsecure();
  // //  client.setCACert(root_ca);  // ✅ use real certificate
  // Serial.println("Connecting to api.openai.com ...");
  // if (!client.connect("api.openai.com", 443)) {
  //   return "❌ Connection failed";
  // }
  // Serial.println("Connected!");

  //   // ---- HTTP request ----
  // client.println("POST /v1/chat/completions HTTP/1.1");
  // client.println("Host: api.openai.com");
  // client.println("Authorization: Bearer " + openaiKey);
  // client.println("Content-Type: application/json");
  // client.println("Content-Length: " + String(strlen(jsonString)));
  // client.println("Connection: close");
  // client.println();
 
  // client.print(jsonString);

  //   if(num_of_image_iterations > 0)
  // {
  //   for(int i = 0; i < num_of_image_iterations; i ++)
  //   {
  //     client.write((uint8_t*)(jsonString + (i * 2000)), 2000);
  //     delay(100);
  //   }

  //   if(remaining_image_len > 0)
  //   {
  //     client.write((uint8_t*)(jsonString + (num_of_image_iterations * 2000)), remaining_image_len);
  //   }

  // }
  // else
  // {
  // client.write((uint8_t*)jsonString, image_string_len);  // binary audio
  // }

  // // Serial.println("428");

  // // ---- Read response ----
  // String response;
  // unsigned long timeout = millis();
  // while (client.connected() || client.available()) {
  //   if (client.available()) {
  //     response += client.readStringUntil('\n');
  //     timeout = millis(); // reset watchdog
  //   }
  //   if (millis() - timeout > SERVER_RESP_TIMEOUT_MS) {  // 10s timeout
  //     break;
  //   }
  // }
  // Serial.println("exiting image transcription");

  // client.stop();
  // return response;
  // #endif
  // Free memory
  // cJSON_Delete(root);
  //Transmit Image_JSON 
  HTTPClient http;
  http.setTimeout(60000);
  http.useHTTP10(true);
  http.begin("https://api.openai.com/v1/" + endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + openaiKey);
  int httpCode = http.POST(jsonString);
  String response = "";
  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP_ERROR : ");
    Serial.println(httpCode);
    log_e("HTTP_ERROR: %d", httpCode);
  }else{
    response = http.getString();
    Serial.println("HTTP REQ SUCCESSFUL");
    // Serial.print("HTTP RESPONSE : ");
    // Serial.println(response);
  }
  //
  http.end();

  return response;

}


#ifdef to_be_done_later
String parse_response(String input)
{
  int index_val = input.indexOf("\"text\"");
  // String text_value = input.substring()
  const char *json_value;
  json_value = input.c_str();
  String transcripted_text = "";
  for(int i = (index_val + 8); i <= 100; i ++)
  {
    if(json_value[i] != '"')
    {
    transcripted_text += String(json_value[i]);
    }
    else
    {
      break;
    }

  }
  Serial.print("Transcripted_text : ");
  Serial.println(transcripted_text);

}
#endif

