#include <WiFi.h>
#include <Config.h>
#include <ESP8266WiFi.h>

#define MAX_ATTEMPTS 20

void connectToWiFi() {

  // Keep trying both networks until one is connected.
  while (WiFi.status() != WL_CONNECTED) {
    if (connectToWiFiMulti(ssid1, password1)) break;
    WiFi.disconnect();

    if (connectToWiFiMulti(ssid2, password2)) break;
    WiFi.disconnect();
  }

  // Added to help if WiFi goes down. https://randomnerdtutorials.com/solved-reconnect-esp8266-nodemcu-to-wifi/
  // The problem appears to be the BT Smart Hub 2.
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // https://github.com/esp8266/Arduino/issues/5083
}

bool connectToWiFiMulti(const char* ssid, const char* password) {
  Serial.printf("\n\nConnecting to WiFi %s", ssid);

  WiFi.begin(ssid, password);

  // Make MAX_ATTEMPTS attempts before trying the other network.
  int Attempt = 0;
    
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++Attempt > MAX_ATTEMPTS) return false;
  }

  Serial.printf("\nConnected to %s\n", ssid);
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  return true;
}
