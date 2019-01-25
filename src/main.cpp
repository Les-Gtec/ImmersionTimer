#include <Arduino.h>
#include <PubSubClient.h>
#include "credentials.h"
#include "pinOutDefinitions.h"
#include "globalVariables.h"
#include "oneWireFunctions.h"
#include "wifiFunctions.h"
#include "mqttFunctions.h"
#include "SwitchPack.h"


//declare global variable starting values
String temp_c_str = "";
char temp_c_char[6];
float setTemp = 50;
float currentTemp = 10;
boolean immersionOn = false;
boolean immersionBath = false;
const long interval = 7000;
unsigned long currentMillis2;

// declare buttons
Click immersionOnSwitch(IMMERSION_ON_BTN, PULLUP);
Click immersionBathSwitch(IMMERSION_BATH_BTN, PULLUP);

void setup(void)
{
  pinMode(IMMERSION_ON_PIN, OUTPUT);
  digitalWrite(IMMERSION_ON_PIN, HIGH); // the relay board has high as off
  pinMode(IMMERSION_BATH_PIN, OUTPUT);
  digitalWrite(IMMERSION_BATH_PIN, HIGH); // the relay board has high as off
  immersionOnSwitch.begin();
  immersionBathSwitch.begin();
  Serial.begin(9600);
  delay(10);
  // Connect to Wi-Fi network
  setup_wifi();
  // Connect to MQTT Broker
  setup_mqtt();
}


void loop(void)
{
  if (immersionOnSwitch.clicked()) {
    immersionOn=!immersionOn;
  }
  if (immersionBathSwitch.clicked()) {
    immersionBath=!immersionBath;
  }
  currentMillis2 = millis();
  static long currentMillis;
  if (currentMillis2 - currentMillis >= interval)
   {
     currentTemp = getOneWireTemp();
     if (currentTemp > 0){
       dtostrf(currentTemp,6,2,temp_c_char);
       temp_c_str = String(currentTemp);
       //temp_c_str.toCharArray(temp_c_char, 6);
       Serial.print("  temperature from string = ");
       Serial.print(temp_c_str);
       Serial.println(" Celsius ");
       Serial.print("  temperature from string array = ");
       Serial.print(temp_c_char);
       Serial.println(" Celsius ");
     }

     currentMillis = currentMillis2;
   }
  if (!mqtt_client.connected()) {
    mqttReconnect();
  }
  mqtt_client.loop();
  if(immersionOn){
    if(currentTemp < setTemp){
      digitalWrite(D4, LOW);
      digitalWrite(D5, HIGH);
    } else {
      digitalWrite(D4, HIGH);
      digitalWrite(D5, LOW);
    }
  } else {
    digitalWrite(D4, HIGH);
    digitalWrite(D5, LOW);
  }
  if(immersionBath){
      digitalWrite(D2, LOW);
      digitalWrite(D7, HIGH);
    } else {
      digitalWrite(D2, HIGH);
      digitalWrite(D7, LOW);
    }

}
