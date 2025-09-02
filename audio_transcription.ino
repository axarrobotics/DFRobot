#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "sample.h"   // generated with xxd -i

// Replace with your credentials
const char* ssid = "Test";
const char* password = "Test123@";
String openaiKey = "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";

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

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  String result = openAI_transcribe();
  Serial.println("=== OpenAI Response ===");
  Serial.println(result);
}

void loop() {
  // nothing
}

String openAI_transcribe() {
  WiFiClientSecure client;

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
  int contentLength = bodyStart.length() + sample_wav_len + bodyEnd.length();

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
  client.write(sample_wav, sample_wav_len);  // binary audio
  client.print(bodyEnd);

  // ---- Read response ----
  String response;
  unsigned long timeout = millis();
  while (client.connected() || client.available()) {
    if (client.available()) {
      response += client.readStringUntil('\n');
      timeout = millis(); // reset watchdog
    }
    if (millis() - timeout > 30000) {  // 10s timeout
      break;
    }
  }

  client.stop();
  return response;
}

