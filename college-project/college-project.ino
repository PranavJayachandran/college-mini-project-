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
#include <DHT.h>
#include <Arduino_JSON.h>
#define DHT_SENSOR_PIN  D7 // The ESP8266 pin D7 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// WiFi parameters to be configured
WiFiServer server(80);
const char* ssid = "realme 8 5G"; // Write here your router's username
const char* password = "12345678"; // Write here your router's passward
const char* SERVER_IP="http://192.168.70.250:3000/sprinkler/predict";
const char* GET_WATER="http://192.168.70.250:3000/sprinkler/numberofsprinklerOn";
String openWeatherMapApiKey = "8112f068a8a7c56dfb2d10b9daadaade";
String my_city = "Trivandrum"; //specify your city
String my_country_code = "IN"; //specify your country code
int relayInput=2;
String json_array;
String sprinklerID="65f09634427352d91ba19443";
String userId="65f09627427352d91ba1943c";
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
String header;
bool sent=0;

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}"; 

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

int getTime(){
  int amount =0;
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, GET_WATER);  // HTTP
    http.addHeader("Content-Type", "application/json");
    int Temperature=10;
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("{\"temperature\":\"" + String(Temperature,2) + "\"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        amount = http.getString().toInt();
      }
    } else {
      Serial.printf("[HTTP] POST... 2 failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  Serial.print("Called");
  Serial.println(amount);
  return amount;
}
void setup(void)
{ 
  pinMode(relayInput, OUTPUT); 
  digitalWrite(relayInput, HIGH); //Switched the led off initially 
  Serial.begin(9600); // initialize the DHT sensor
   dht_sensor.begin();
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.println(WiFi.localIP());
  server.begin();
}
float sense()
{
  float temperature_C = dht_sensor.readTemperature();
  return temperature_C;
}
void sendData(){

    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, SERVER_IP);  // HTTP
    http.addHeader("Content-Type", "application/json");
    float Temperature=sense();
    Serial.print(Temperature);
    Serial.println(getWeather());
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("{\"temperature\":\""+String(Temperature,2)+"\",\"weather\":\""+getWeather()+"\",\"sprinklerID\":\""+sprinklerID+"\",\"userId\":\""+userId+"\"}");


    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}
int getWeather()
{
    String server = "http://api.openweathermap.org/data/2.5/weather?q=" + my_city + "," + my_country_code + "&APPID=" + openWeatherMapApiKey;
      json_array = httpGETRequest(server.c_str());
      JSONVar my_obj = JSON.parse(json_array);
      if (JSON.typeof(my_obj) == "undefined") {
        Serial.println("Parsing input failed!");
        return 0;
      }
      return my_obj["weather"][0]["id"];
}
void irrigate(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then                  // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /on") >= 0) {
              Serial.println("GPIO 5 on");
              int time=getTime();
              digitalWrite(relayInput, LOW);
	            delay(time);
	            digitalWrite(relayInput, HIGH); 
            }
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    if(!sent)
    {
      sendData();
      sent=true;
    }
  }
   irrigate();
}