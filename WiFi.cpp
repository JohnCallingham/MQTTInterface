#include <WiFi.h>
#include <Config.h>
#include <ESP8266WiFi.h>

// ssid and password are defined in Config.h

void connectToWiFi() {
  Serial.print("\n\nConnecting to WiFi ");

  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nConnected to %s\n", ssid);
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  // Added to help if WiFi goes down. https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
  // The problem appears to be the BT Smart Hub 2.
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // https://github.com/esp8266/Arduino/issues/5083

}
