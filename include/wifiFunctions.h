#include <Arduino.h>
#include <ESP8266WiFi.h>

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWD;

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
