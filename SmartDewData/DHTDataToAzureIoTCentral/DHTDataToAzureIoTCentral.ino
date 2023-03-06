// SmartDew Project File
// Senior Design Team 21
// FIU College of Engineering and Computing
//    * Edward Heeren (Team Lead)
//    * Keith Anderson
//    * Linval Bailey
//    * Ravi Pooran
//    * Alex Zorrilla
//
// This code runs on a NodeMCU ESP8266 with a DHT22 module attached
// It uses the WifiManager library to enable user to add their own wifi
// MQTT data pushes information to the cloud

//#include <ESP8266WiFi.h>
// Adafruit_MQTT library https://github.com/adafruit/Adafruit_MQTT_Library
#include <Adafruit_MQTT.h>
// DHT sensor library by Adafruit https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>
// WiFiManager library https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>

// No longer using

// #include "iotc/common/string_buffer.h"
// #include "iotc/iotc.h"

// DHT data pin D6
#define DHTPIN D6

// Using a DHT22 module
#define DHTTYPE DHT22 

// no longer using
// #define WIFI_SSID "Alex"
// #define WIFI_PASSWORD "blackops124"
 
const char* ID_SCOPE = "0ne00865AD6";
const char* DEVICE_ID = "dht22";
const char* PRIMARY_KEY = "AhUxWspmnfCztec1DoMNws7jGLi78zPdoDf6cv7Ig6o=";
 
DHT dht(DHTPIN, DHTTYPE);
 
//void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo);
//#include "connection.h"
 
// No longer using Azure, temporarily commenting out

/*
void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo) {
  // ConnectionStatus
  if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0) {
    LOG_VERBOSE("Is connected ? %s (%d)",
                callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO",
                callbackInfo->statusCode);
    isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
    return;
  }
 
  // payload buffer doesn't have a null ending.
  // add null ending in another buffer before print
  AzureIOT::StringBuffer buffer;
  if (callbackInfo->payloadLength > 0) {
    buffer.initialize(callbackInfo->payload, callbackInfo->payloadLength);
  }
 
  LOG_VERBOSE("- [%s] event was received. Payload => %s\n",
              callbackInfo->eventName, buffer.getLength() ? *buffer : "EMPTY");
 
  if (strcmp(callbackInfo->eventName, "Command") == 0) {
    LOG_VERBOSE("- Command name was => %s\r\n", callbackInfo->tag);
  }
}*/
 
void setup() {
  Serial.begin(9600);
  Serial.println("Init");

  // Creating the wifi manager
  WiFiManager wifiManager;

  //This is the name of the ESP8266 Wifi network
  //Connect to ESP8266 then save user WIFI info
  //Default gateway IP = 192.168.4.1
  wifiManager.autoConnect("SmartDew-Wifi");

  // Start dht connection
  dht.begin();

  //Using WiFiManager, the following should be depricated
  //connect_wifi(WIFI_SSID, WIFI_PASSWORD);
  //connect_client(ID_SCOPE, DEVICE_ID, PRIMARY_KEY);
 
  /* if (context != NULL) {
    lastTick = 0;  // set timer in the past to enable first telemetry a.s.a.p
  } */ 


} // end setup
 
void loop() {
 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.printf("Humidity: %f || Temperature: %f/n", h, t);
  Serial.println();


  // Need to re-implement MQTT connection
  /*
  if (isConnected) {
 
    unsigned long ms = millis();
    if (ms - lastTick > 10000) {  // send telemetry every 10 seconds
      char msg[64] = {0};
      int pos = 0, errorCode = 0;
 
      lastTick = ms;
      if (loopId++ % 2 == 0) {  // send telemetry
        pos = snprintf(msg, sizeof(msg) - 1, "{\"Temperature\": %f}",
                       t);
        errorCode = iotc_send_telemetry(context, msg, pos);
        
        pos = snprintf(msg, sizeof(msg) - 1, "{\"Humidity\":%f}",
                       h);
        errorCode = iotc_send_telemetry(context, msg, pos);
          
      } else {  // send property
        
      } 
  
      msg[pos] = 0;
 
      if (errorCode != 0) {
        LOG_ERROR("Sending message has failed with error code %d", errorCode);
      }
    }
 
    iotc_do_work(context);  // do background work for iotc
  } else {
    iotc_free_context(context);
    context = NULL;
    connect_client(ID_SCOPE, DEVICE_ID, PRIMARY_KEY);
  }
  */
  delay(2000);
} // end loop
