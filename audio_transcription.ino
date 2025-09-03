#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "sample.h"   // generated with xxd -i
#include "wav_header.h"
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ESP_I2S.h"
#define BUTTON_PIN 0
#define LED_PIN 3
#define SAMPLE_RATE     (4000) // The sample rate is tested with OpenAI and selected. 
#define DATA_PIN        (GPIO_NUM_39)
#define CLOCK_PIN       (GPIO_NUM_38)

//Other IMP MACROS
#define SERVER_RESP_TIMEOUT_MS 40000 
//For I2S_audio
I2SClass I2S;
bool isRecording = false; 
uint8_t *wavBuffer = NULL;
size_t wavBufferSize = 0; 

// Replace with your credentials
const char* ssid = "Test";
const char* password = "Test123@";
String openaiKey = "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";
int num_of_audio_iterations = 0;
int remaining_audio_len = 0;
String result;

#ifdef test
// ISRG Root X1 certificate (valid for api.openai.com)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgISA6UjtpRwaP1pY9lKGVt7lU5RMA0GCSqGSIb3DQEBCwUA\n"
"MEoxCzAJBgNVBAYTAlVTMRMwEQYDVQQKDApMZXQncyBFbmNyeXB0MSMwIQYDVQQD\n"
"DBpMZXQncyBFbmNyeXB0IFJvb3QgQ0EgWDEwHhcNMjAwNDE1MDAwMDAwWhcNMjMw\n"
"NTExMTIwMDAwWjA6MQswCQYDVQQGEwJVUzETMBEGA1UECgwKTGV0J3MgRW5jcnlw\n"
"dDEjMCEGA1UEAwwaTGV0J3MgRW5jcnlwdCBSb290IENBIFgxMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzNPdF2dAhtqgWpyfcU5q5o8rZg1aI5WJxks/\n"
"3u7QzTz1U5WZty4LYtL3zM3vG+6ZChqPOeTyy0MMHQhdGjw6a5zXgtrZD2jE3j9t\n"
"H+Q6h/4gkXWm+dc6ZgIGpTuH/uhV7KaKnWXexsb6LSSUnk1wUFV0UodWIaUgVRa3\n"
"nsi2MJl2UuQMSiYq8rrfNIThO3WAnV4Tp5z5vI3oLShM2ZkH01CM4Dk5W7vY6D0T\n"
"mkr1+8PIFzH5o2h3iFz6yC0+9F7GeyXKaqcrX8iFMSmI5PgZsQCKA+J4sF+PP7oM\n"
"6e3hUjBfGfRKnQ60r1lWB3yZlIUV73pR/3nGMyGH0z3gH4S4RwIDAQABo4ICZzCC\n"
"AmMwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcD\n"
"AjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBSoSmpjBH3DU7g79IYz1n34RBduvjAf\n"
"BgNVHSMEGDAWgBQULrMXt1hWy65QCUDmH6+dixTCxjA4BggrBgEFBQcBAQIwMC4G\n"
"CCsGAQUFBzABhiJodHRwOi8vb2NzcC5pbnQtbGV0c2VuY3J5cHQub3JnMIIBAwYI\n"
"KwYBBQUHAQsEgfowgfcwKwYIKwYBBQUHMAGGH2h0dHA6Ly9vY3NwLmludC1sZXRz\n"
"ZW5jcnlwdC5vcmcwNQYIKwYBBQUHMAKGKWh0dHA6Ly9jZXJ0LmNhY2VydC5vcmcv\n"
"bGV0c2VuY3J5cHQucm9vdHgubmV3LmNydDA/BggrBgEFBQcwAoYzaHR0cDovL2Nh\n"
"Y2VydC5pbnQtbGV0c2VuY3J5cHQub3JnL2xldHNlbmNyeXB0LXJvb3QueDEuY2Vy\n"
"MEUGA1UdIAQ+MDwwOgYEVR0gADAyMDAGCCsGAQUFBwIBFiRodHRwczovL2xldHNl\n"
"bmNyeXB0Lm9yZy9yZXBvc2l0b3J5LzA8BgNVHR8ENTAzMDGgL6AthitodHRwOi8v\n"
"Y3JsLmludC1sZXRzZW5jcnlwdC5vcmcvbGV0c2VuY3J5cHQtcjEuY3JsMA0GCSqG\n"
"SIb3DQEBCwUAA4IBAQAfjYONglz8q6SyQHhX+3vSeKywvwwUMN5D0ZDKkuEv/Lnl\n"
"AJZZHYhHBGQFHoJ0L3hPiS14WraUQchSUg36Xx2DJBgPKZW/zRLvcmjO2VXfDgeU\n"
"90PY9l00ZVY0Dw+dYr4fZb+mT0hxQY11rIHTU6IY6PkUvPoUVeKq8YGHeA8lHk5Q\n"
"ZGc7ISZdkV73Xx/KuD3W9tA1XkxTY6EfdM4DNXVutYPzmzMkmJ8th6Ep+bI7AN0V\n"
"2XtsYUrpMhuJB+HtjCgLe3H8/3KwVjFuS7Qn2V/gP0m3C5ZPQ66n8B2CNkeJq8Jp\n"
"TAxJ/dfAyNqFu8lfFWsByLbnL9XVrWYaM3CeYs+m\n"
"-----END CERTIFICATE-----\n";
#endif

void setup() {
  Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  audioInit();

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  //  result = openAI_transcribe((uint8_t *)sample_wav, sample_wav_len);
  // Serial.println("=== OpenAI Response ===");
  // Serial.println(result);
}

void audioInit()
{
  I2S.setPinsPdmRx(CLOCK_PIN, DATA_PIN);
  if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("Failed to initialize I2S PDM RX");
  }
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
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

void stopRecording() {
    if (!isRecording) return;

    isRecording = false;
    
    Serial.printf("Stop recording. Total recorded size: %u\n", wavBufferSize);
    // Serial.println("audio_data_start : ");
    // for(int i = 0; i < wavBufferSize; i ++)
    // {
    //   Serial.print("0x");
    //   Serial.print(*(wavBuffer + i), HEX);
    //   Serial.print(", ");
    //   if(i != 0)
    //   {
    //   if((i % 20) == 0)
    //   {
    //     Serial.println();
    //   }
    //   }
    // }
    // Serial.println();

    pcm_wav_header_t *header = (pcm_wav_header_t *)wavBuffer;
    header->descriptor_chunk.chunk_size =  (wavBufferSize) + sizeof(pcm_wav_header_t) - 8;
    header->data_chunk.subchunk_size = wavBufferSize - PCM_WAV_HEADER_SIZE;

    Serial.println("Start speech to text! ");
    //For Testing
    // wavBuffer = (uint8_t *)sample_wav;
    // wavBufferSize = 7724;
    //     Serial.println("audio_data_start : ");
    // for(int i = 0; i < 50; i ++)
    // {
    //   Serial.print("0x");
    //   Serial.print(*(wavBuffer + i), HEX);
    //   Serial.print(", ");
    //   if(i != 0)
    //   {
    //   if((i % 20) == 0)
    //   {
    //     Serial.println();
    //   }
    //   }
    // }
    // Serial.println();

  String txt = openAI_transcribe(wavBuffer, wavBufferSize);
  Serial.println("=== OpenAI Response ===");
  Serial.println(txt);
  // parse_response(txt);

    // free(wavBuffer);
    // if(txt!=NULL){
    //     Serial.printf("SpeechToText Message:\n%s\n", txt.c_str());
    //     Serial.println("Start image Answering! ");
    //     txt=imageAnswering(txt);
    //     if(txt!=NULL){
    //       Serial.printf("imageAnswering Message:\n%s\n", txt.c_str());
    //       Serial.println("Start text to audio! ");
    //       if(TextToSpeech(txt)==-1){
    //         Serial.println("Audio reception failed! ");
    //       }
    //     }else{
    //       Serial.println("imageAnswering failed!");
    //     }
    // }else{
    //   Serial.println("speech to text failed!");
    // }
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
    return "❌ Connection failed";
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
  String response;
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

