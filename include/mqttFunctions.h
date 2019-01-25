#include <Arduino.h>
#include "credentials.h"
#include "globalVariables.h"

const char* mqtt_server_add = MQTT_SERVER_ADD;
const int   mqtt_server_port = MQTT_SERVER_PORT;
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void sendImersionTemp(){
  Serial.print("Publish message: ");
  Serial.println(currentTemp);
  mqtt_client.publish("immersion-temp", temp_c_char);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
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

void mqttReconnect() {
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


void setup_mqtt() {
  mqtt_client.setServer(mqtt_server_add, mqtt_server_port);
  mqtt_client.setCallback(mqttCallback);
}
