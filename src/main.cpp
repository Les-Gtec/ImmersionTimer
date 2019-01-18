#include <Arduino.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"


// Set Up OneWire
OneWire  ds(D3);  // on pin D3 (a 4.7K resistor is necessary)

//Set up network and MQTT
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWD;
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


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to...");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi connected successfully");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
}


float getTemp(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;

  if ( !ds.search(addr))
  {
    ds.reset_search();
    delay(250);
    return 0;
  }

  if (OneWire::crc8(addr, 7) != addr[7])
  {
      Serial.println("CRC is not valid!");
      return 0;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  switch (addr[0])
  {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return 0;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1000);
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++)
  {
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10)
    {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms

  }
  celsius = (float)raw / 16.0;
  Serial.print("  IN GETTemp Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius ");
  return celsius;
}

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
     currentTemp = getTemp();
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
