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
float setTemp = 50;
float currentTemp = 10;
boolean immersionOn = false;
boolean immersionBath = false;
const long interval = 7000;
const int buttonInterval = 400; // number of millisecs between button readings
unsigned long currentMillis2 = 0;
unsigned long previousImmersionButtonMillis = 0; // time when button press last checked
unsigned long previousBathButtonMillis = millis(); // time when button press last checked


void setup(void)
{
  pinMode(IMMERSION_ON_PIN, OUTPUT);
  digitalWrite(IMMERSION_ON_PIN, HIGH); // the relay board has high as off
  pinMode(IMMERSION_BATH_PIN, OUTPUT);
  digitalWrite(IMMERSION_BATH_PIN, HIGH); // the relay board has high as off
  pinMode(IMMERSION_ON_BTN, INPUT_PULLUP);
  pinMode(IMMERSION_BATH_BTN, INPUT_PULLUP);
  Serial.begin(9600);
  delay(10);
  // Connect to Wi-Fi network
  setup_wifi();
  // Connect to MQTT Broker
  setup_mqtt();
}
void readImmersionButton() {

  if (currentMillis2 - previousImmersionButtonMillis >= buttonInterval) {

    if ((digitalRead(IMMERSION_ON_BTN) == LOW)) { // && !immersionOn
      Serial.println(" Immersion On BTN Pressed ");
      //immersionOn = true;
      previousImmersionButtonMillis = currentMillis2 + buttonInterval;
    }
  }

}
void readBathButton() {
  unsigned long timeDifference = currentMillis2 - previousBathButtonMillis;
  if (timeDifference >= buttonInterval) {
    if (digitalRead(IMMERSION_BATH_BTN) == LOW) {
      Serial.print(" Immersion Bath BTN Pressed current millis: " );
      Serial.print(currentMillis2);
      Serial.print(" button Interval: ");
      Serial.print(buttonInterval);
      Serial.print(" previous millis: ");
      Serial.println(previousBathButtonMillis);
      Serial.print("timeDifference: ");
      Serial.println(timeDifference);
      //immersionBath = !immersionBath;
      previousBathButtonMillis = currentMillis2 + buttonInterval;
    }
  }

}


void loop(void)
{
  currentMillis2 = millis();
  readImmersionButton();
  readBathButton();
  static long currentMillis;
  if (currentMillis2 - currentMillis >= interval)
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
