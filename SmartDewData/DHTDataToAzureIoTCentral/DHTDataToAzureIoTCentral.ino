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

// #include <ESP8266WiFi.h>
// Adafruit_MQTT library https://github.com/adafruit/Adafruit_MQTT_Library
#include <Adafruit_MQTT.h>
// DHT sensor library by Adafruit https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>
// WiFiManager library https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

// Azure code header files in iots directory
#include "src/iotc/common/string_buffer.h"
#include "src/iotc/iotc.h"

// HCSR04 Sensor Library https://github.com/gamegine/HCSR04-ultrasonic-sensor-lib
#include <HCSR04.h>
HCSR04 hc(13, 15); // Initialize Pin D7, D8

// No longer using
// #include "iotc/common/string_buffer.h"
// #include "iotc/iotc.h"

// DHT data pin D6
#define DHTPIN D6

// Using a DHT22 module
#define DHTTYPE DHT22 

// Interrupt pin (GPIO16 on ESP8266)
#define INTERRUPT_PIN D0

// Turning this pin on activates the water pump
#define MOTOR_PIN D1

const char* ID_SCOPE = "0ne009BAAEA";
const char* DEVICE_ID = "1cs5dwz3pwz";
const char* PRIMARY_KEY = "ii4RXJzO/2tfzHhsmnhmWjPJoRJfDooLbogCoxDZaLQ=";

const float waterVolumeMultiplier = 3.1415 * 25.8; // pi * r^2 * dist = cylindar volume 

const uint32 INTERRUPT_PERIOD = 10*1000000;

// Pressure Sensor Consts
const int pressureInput = A0; //select the analog input pin for the pressure transducer
const float pressureZero = 102.4; //analog reading of pressure transducer at 0psi
const float pressureMax = 348.16; //analog reading of pressure transducer at 100psi
const int pressuretransducermaxPSI = 30; //psi value of transducer being used
const int sensorreadDelay = 250; //constant integer to set the sensor read delay in milliseconds

float pressureValue = 0; //variable to store the value coming from the pressure transducer
 
DHT dht(DHTPIN, DHTTYPE);
 
void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo);
#include "src/connection.h"


void on_event(IOTContext ctx, IOTCallbackInfo* callbackInfo) 
{
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
}

/*
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
*/


/*
  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
 */  
 
// Global flag to prevent the device from going to sleep while the motor is running.
bool motorOn = false;

void setup() {
  Serial.begin(115200);
  while(!Serial){}  // Wait for serial to initialize
  Serial.println("\nInit");

  pinMode(MOTOR_PIN, OUTPUT);

  // Creating the wifi manager
  WiFiManager wifiManager;

  //This is the name of the ESP8266 Wifi network
  //Connect to ESP8266 then save user WIFI info
  //Default gateway IP = 192.168.4.1
  wifiManager.autoConnect("SmartDew-Wifi");

  // Start dht connection
  dht.begin();

  if (context != NULL) {
    lastTick = 0;  // set timer in the past to enable first telemetry a.s.a.p
  }

} // end setup
 
void loop() {
  Serial.println("interrupt activated");

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float dist = hc.dist();

  Serial.printf("Humidity: %f || Temperature: %f || Distance: %f", h, t, dist);
  Serial.println();

  // Run motor when distance is more than 30 cm
  while(dist > 30)
  { 
    motorOn = true;
    Serial.printf("Dist = %f, motor on!\n", dist);
    analogWrite(MOTOR_PIN, 128);
    dist = hc.dist();
    delay(1000);
  }
  if(dist <= 30)
  {
    motorOn = false;
    Serial.println("Motor off");
    analogWrite(MOTOR_PIN, 0);
  }

  // Azure connection code
  if (isConnected) 
  {
    unsigned long ms = millis();

    char msg[64] = {0};
    int pos = 0, errorCode = 0;

    lastTick = ms;
    Serial.printf("LoopID: %d", loopId);
    if (loopId++ % 2 == 0) // send telemetry
    {  
      pos = snprintf(msg, sizeof(msg) - 1, "{\"Temperature\": %f}",
                      t);
      errorCode = iotc_send_telemetry(context, msg, pos);
      Serial.println("Temp sent");

      pos = snprintf(msg, sizeof(msg) - 1, "{\"Humidity\":%f}",
                      h);
      errorCode = iotc_send_telemetry(context, msg, pos);
      Serial.println("Humidity sent");

      pos = snprintf(msg, sizeof(msg) - 1, "{\"WaterVolume\":%f}",
                      dist);
      errorCode = iotc_send_telemetry(context, msg, pos);
      Serial.println("WaterLevel sent");

    } else {  // send property
      
    } 

    msg[pos] = 0;

    if (errorCode != 0) {
      LOG_ERROR("Sending message has failed with error code %d", errorCode);
    }
    iotc_do_work(context);  // do background work for iotc
  } else 
  {
    iotc_free_context(context);
    context = NULL;
    connect_client(ID_SCOPE, DEVICE_ID, PRIMARY_KEY);
  }

  // rst pin (GPIO16) should be connected to D0, but only after programming or it won't flash. Connect switch?
  if(!motorOn)
  {
    ESP.deepSleep(INTERRUPT_PERIOD); 
  }
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
} // end loop
