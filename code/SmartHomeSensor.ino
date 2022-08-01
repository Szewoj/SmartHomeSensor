/** SmartHomeSensor
 *    Project for reading measurments from DHT11 and light sensor
 *    on ESP8266 development board and transmitting data to ThingSpeak
 *    cloud channel.
 *    
 *  Author: Wojciech Szerszeń
 */

//--- VERSION ---
//* <-- version controlled by this sign: [/*] debug; [//*] release
#define RELEASE
/*
*/

//--- INCLUDES ---
#include "DHT.h"          // DHT11 sensor managment
#include <ESP8266WiFi.h>  // Network menagement
#include <ThingSpeak.h>   // ThingSpeak communication
#include "keys.h"         // sensitive defines
//---

//--- SETUP VALUES ---
#define   LEDPIN    2
#define   DHTPIN    D5
#define   DHTTYPE   DHT11
#define   LSPIN     A0
#define   BAUD      115200
#define   MDEL      5995      // measurement delay (ms)
#define   SDEL      2995      // special delay (ms)
#define   MAX_H_IT  1         // max humidity iterator
#define   MAX_LS_IT 4         // max light sensor iterator

#define   WIFISSID  KEYS_SSID
#define   PASSKEY   KEYS_PASSKEY
#define   TSADDR    "api.thingspeak.com"  // ThingSpeak api address
#define   CHNID     KEYS_CHANNELID        // ThinkSpeak channel id
#define   WKEY      KEYS_APIWRITE         // Write key for channel
#define   RKEY      KEYS_APIREAD          // Read key for channel
//---

//--- GLOBAL VARIABLES ---
unsigned int alIter = 0;     // light sensor iterator
unsigned int hIter = 0;      // humidity iterator
float lightVolt[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float humidity[2] = {0.0, 0.0};
float temperature = 0.0;

int transferStatus = 406;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
//---

//--- FUNCTIONS ---

float getVoltsFromAdc(int v) {
  return 3.3 * (float)v / 1023.0;
}

int connectWiFi(){
  WiFi.begin(WIFISSID, PASSKEY);
  
  #ifndef RELEASE
  Serial.println("\nConnecting to WiFi");
  #endif
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LEDPIN, LOW);
    delay(1000);
    #ifndef RELEASE
      Serial.print(".");
    #endif
    digitalWrite(LEDPIN, HIGH);
  }
  #ifndef RELEASE
    Serial.println( "\r\nConnected" );
  #endif
  
  ThingSpeak.begin(client);
  return 0;
}

int writeTSData( float* ambLight, float* humi, float temp ){
  ThingSpeak.setField( 1, humi[1] );
  ThingSpeak.setField( 2, humi[0] );
  ThingSpeak.setField( 3, temp );
  ThingSpeak.setField( 4, ambLight[4] );
  ThingSpeak.setField( 5, ambLight[3] );
  ThingSpeak.setField( 6, ambLight[2] );
  ThingSpeak.setField( 7, ambLight[1] );
  ThingSpeak.setField( 8, ambLight[0] );
  
  int writeSuccess = ThingSpeak.writeFields( CHNID, WKEY );
  return writeSuccess;
}

void printPlotData() {
  #ifndef RELEASE
    Serial.print("Light:"); Serial.print(lightVolt[MAX_LS_IT]*10+20); Serial.print(" ");
    Serial.print("Temp:"); Serial.print(temperatureC[MAX_T_IT]); Serial.print(" ");
    Serial.print("Humi:"); Serial.print(humidity); Serial.print(" ");
    Serial.print("Status:"); Serial.print(transferStatus/10.0); Serial.print(" ");
    Serial.println("");
  #endif
}

void setup() { //setup code
  #ifndef RELEASE
  Serial.begin(BAUD);
  #endif
  
  delay(10);

  pinMode(LEDPIN, OUTPUT);
  connectWiFi();
  dht.begin();
}

void loop() { //loop code

  digitalWrite(LEDPIN, LOW);

  if(alIter == MAX_LS_IT) {
    // measurements:
    lightVolt[MAX_LS_IT] = getVoltsFromAdc(analogRead(LSPIN));
    humidity[MAX_H_IT] = dht.readHumidity();
    temperature = dht.readTemperature();

    //ThingSpeak
    transferStatus = writeTSData(lightVolt, humidity, temperature);

    printPlotData();

    digitalWrite(LEDPIN, HIGH);
    delay(MDEL);

    alIter = 0;
    hIter = 0;

  } else {

    lightVolt[alIter] = getVoltsFromAdc(analogRead(LSPIN));
    digitalWrite(LEDPIN, HIGH);

    if(alIter == 1) {   // measure mid delay temperature (t - 15)
      delay(SDEL);

      digitalWrite(LEDPIN, LOW);

      humidity[hIter] = dht.readHumidity();
      ++hIter;

      digitalWrite(LEDPIN, HIGH);
      delay(SDEL);
    } else {
      digitalWrite(LEDPIN, HIGH);
      delay(MDEL);
    }

    ++alIter;

  }
}

//---
