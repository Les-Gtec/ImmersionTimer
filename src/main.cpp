#include <Arduino.h>
#include <PubSubClient.h>
#include "credentials.h"
#include "pinOutDefinitions.h"
#include "globalVariables.h"
#include "oneWireFunctions.h"
#include "wifiFunctions.h"
#include "mqttFunctions.h"

//declare global variable starting values
String temp_c_str = "";
char temp_c_char[6];
float setTemp = 10;
float currentTemp = 10;
boolean immersionOn = false;
boolean immersionBath = false;
const long interval = 7000;


void setup(void)
{
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
  pinMode(D2, OUTPUT);
  digitalWrite(D2, HIGH);
  pinMode(D7, OUTPUT);
  digitalWrite(D7, LOW);
  Serial.begin(9600);
  delay(10);
  // Connect to Wi-Fi network
  setup_wifi();
  // Connect to MQTT Broker
  setup_mqtt();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish("amAlive", "ImmersionTimer");
      // ... and resubscribe
      mqtt_client.subscribe("immersion-temp-check");
      mqtt_client.subscribe("immersion-on");
      mqtt_client.subscribe("immersion-off");
      mqtt_client.subscribe("immersion-bath");
      mqtt_client.subscribe("immersion-sink");

    } else {
      Serial.print("failed,");
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(void)
{
  static long currentMillis;
  if (millis() - currentMillis >= interval)
   {
     currentTemp = getOneWireTemp();
     if (currentTemp > 0){
       temp_c_str = String(currentTemp);
       temp_c_str.toCharArray(temp_c_char, 6);
       Serial.print("  temperature from string = ");
       Serial.print(temp_c_str);
       Serial.println(" Celsius ");
       Serial.print("  temperature from string array = ");
       Serial.print(temp_c_char);
       Serial.println(" Celsius ");
     }

     currentMillis = millis();
   }
  if (!mqtt_client.connected()) {
    reconnect();
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
