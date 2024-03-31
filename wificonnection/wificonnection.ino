/*
 * Connects the ESP8266 NodeMCU board to wifi and prints the IP address
 * 
 * by:
 * ROBOSANS
 * based on ThingSpeak Library example
 * 
 * https://www.robosans.com/
 */

#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h> 

// WiFi parameters to be configured
const char* ssid = "realme 8 5G"; // Write here your router's username
const char* password = "12345678"; // Write here your router's passward
const char* SERVER_IP="http://192.168.70.250:4000/api";

void setup(void)
{ 
  Serial.begin(9600);
  // Connect to WiFi
  WiFi.begin(ssid, password);



  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.println(WiFi.localIP());

}
void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, SERVER_IP);  // HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("{\"hello\":\"world\"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}