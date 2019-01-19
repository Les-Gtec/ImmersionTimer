#include <Arduino.h>
#include <PubSubClient.h>
#include "credentials.h"
#include "pinOutDefinitions.h"
#include "oneWireFunctions.h"
#include "wifiFunctions.h"

//Set up network and MQTT

const char* mqtt_server_add = MQTT_SERVER_ADD;
const int   mqtt_server_port = MQTT_SERVER_PORT;
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

//set up variables
String temp_c_str = "";
char temp_c_char[6];
float setTemp = 10;
float currentTemp = 10;
boolean immersionOn = false;
boolean immersionBath = false;
const long interval = 7000;

void sendImersionTemp(){
  Serial.print("Publish message: ");
  Serial.println(currentTemp);
  mqtt_client.publish("immersion-temp", temp_c_char);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(currentTemp);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(strcmp(topic, "immersion-on")==0){
    Serial.println("Setting Immersion On");
    immersionOn = true;
    payload[length] = '\0';
    String s = String((char*)payload);
    setTemp = s.toFloat();
    //setTemp = 30;
  }
  if(strcmp(topic, "immersion-off")==0){
    Serial.println("Setting Immersion Off");
    immersionOn = false;
    //setTemp = 30;
  }
  if(strcmp(topic, "immersion-bath")==0){
    Serial.println("Setting Immersion Bath");
    immersionBath = true;
    //setTemp = 30;
  }
  if(strcmp(topic, "immersion-sink")==0){
    Serial.println("Setting Immersion Sink");
    immersionBath = false;
    //setTemp = 30;
  }
  sendImersionTemp();
}


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

  //mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setServer(mqtt_server_add, mqtt_server_port);
  mqtt_client.setCallback(callback);
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
