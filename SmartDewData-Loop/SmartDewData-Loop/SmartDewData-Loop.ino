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
//
//				SmartDew Wiring
//					
//					
//	.........................................
//	!			                    							!
//	!		    	  ------------------------- 	!
//	!	    	  	| 	NODEMCU ESP8266	    |  	! 
//	!           |A0				        	D0	|...!
//	!	      		|RSV        				D1	| -> Motor Control Pin
//	!			      |RSV        				D2	|
//	S     			|SD3        				D3	|
//	P		      	|SD2        				D4	|
//	S			      |SD1        				3V3	|
//	T			      |CMD        				GND	|
//	!		      	|SDO        				D5	|
//	!			      |CLK	        			D6	| <-> DHT22 Com
//	!		      	|GND		         		D7	| <-> JSN-SR04T Rx Trig
//	!		      	|3V3	        			D8	| <-> JSN-SR04T TX Echo
//	!		      	|EN		        			RX	|
//	!...........|RST	        			TX	|
//	PSU 3V3 <-> |GND	        			GND	| <-> gnd bus
//  PSU 3v3 <->	|VIN	        			3V3	| <-> 3v3 bus
//			      	|			            			|
//			      	|O<-RST|MICROUSB|FLSH->O|
//			      	-------------------------
 
#include <ESP8266WiFi.h>
#include "src/iotc/common/string_buffer.h"
#include "src/iotc/iotc.h"
#include "DHT.h"
 
#define DHTPIN D6
 
#define DHTTYPE DHT22 

// WiFiManager library https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
 
const char* ID_SCOPE = "0ne009BAAEA";
const char* DEVICE_ID = "1cs5dwz3pwz";
const char* PRIMARY_KEY = "ii4RXJzO/2tfzHhsmnhmWjPJoRJfDooLbogCoxDZaLQ=";

const int buttonPin = 4;  // the number of the pushbutton pin
 
//const uint32 INTERRUPT_PERIOD = 10*1000000;

DHT dht(DHTPIN, DHTTYPE);
 
int buttonState = 0;  // variable for reading the pushbutton status

void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo);
#include "src/connection.h"
 
void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo) {
  // ConnectionStatus
  if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0) {
    LOG_VERBOSE("Is connected ? %s (%d)",
                callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO",
                callbackInfo->statusCode);
    isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
    LOG_VERBOSE("\n ---PING--- \n");
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
}
 
void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(buttonPin, INPUT);

  // Creating the wifi manager
  WiFiManager wifiManager;

  //This is the name of the ESP8266 Wifi network
  //Connect to ESP8266 then save user WIFI info
  //Default gateway IP = 192.168.4.1
  wifiManager.autoConnect("SmartDew-Wifi");
 
  if (context != NULL) {
    lastTick = 0;  // set timer in the past to enable first telemetry a.s.a.p
  }
   dht.begin();
}
 
void loop() 
{
 
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    // turn LED on:
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
  }

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
  //ESP.deepSleep(INTERRUPT_PERIOD); 
}