// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTTServo.h>
#include <MQTTContainer.h>
#include <WebSocketsServer.h>
#include <MQTTOutput.h>
#include <MQTTInput.h>
#include <Adafruit_PWMServoDriver.h>
#include "Adafruit_MCP23017.h"

const char* ssid = "BT-SCAKPC";
const char* password = "btvxqTve4aPQkr";

WiFiClient wifiClient;
PubSubClient mqttClient;
WebSocketsServer webSocket(81);

// Create a pointer to a PCF8575 object. This is the I2C I/O expander which can be used for sensors and relays.
Adafruit_MCP23017* mcp = new Adafruit_MCP23017();

// Create a pointer to an Adafruit_PWMServoDriver object. This is the I2C PWM expander which will be used for servos.
Adafruit_PWMServoDriver* pwm = new Adafruit_PWMServoDriver(0x40);

// Create an MQTTContainer object which will contain pointers to all the MQTTInput, MQTTOutput and MQTTServo objects.
MQTTContainer container;





// Forward declarations.
void connectToWiFi();

void setup() {
  Serial.begin(115200);

  // Initialise the I/O expander.
  mcp->begin();

  // Initialise the PWM expander.
  pwm->begin();
  pwm->setOscillatorFrequency(27000000);
  pwm->setPWMFreq(50);  // Analog servos run at ~50 Hz updates



  // Create a pointer to an MQTTInput object which is connected to GPIO14 of the 8266.
  MQTTInput* input1 = container.addInput(14, "trains/track/sensor/GPIO14");

  // Create a pointer to an MQTTInput object which is connected to port 8 of the I2C I/O expander.
  MQTTInput* input2 = container.addInput(8, "trains/track/sensor/Port5", mcp);

  // Create a pointer to an MQTTInput object whose ACTIVE message will be changed in later code.
  MQTTInput* input3 = container.addInput(6, "trains/track/turnout/123", mcp);

  // Create a pointer to an MQTTInput object whose ACTIVE message will be changed in later code.
  MQTTInput* input4 = container.addInput(7, "trains/track/turnout/123", mcp);

  // Create a pointer to an MQTTOutput object which is connected to an 8266 GPIO pin.
  //  This will actually turn the built in LED on and off for testing.
  MQTTOutput* output1 = container.addOutput(LED_BUILTIN, "trains/track/light/L001");

  // Create a pointer to an MQTTOutput object which represents an output which is connected to port 14 of the I2C I/O expander.
  //MQTTOutput* output2 = container.addOutput(14, "trains/track/light/L002", mcp);

  
  // Create a pointer to an MQTTServo object which is connected port 0 of the I2C PWM expander.
  MQTTServo* servo1 = container.addServo(0, "trains/track/turnout/123", pwm);

  // Create a pointer to an MQTTServo object which is connected to port 15 of the I2C PWM expander.
  MQTTServo* servo2 = container.addServo(15, "trains/track/turnout/124", pwm);





  connectToWiFi();

  webSocket.begin();

  container.setStartupTopic("events");

  servo1->setClosedTopic("trains/track/sensor/456");
  servo1->setThrownTopic("trains/track/sensor/789");
  servo1->setMidPointTopic("trains/track/light/L001");

  servo1->setAngleClosed(80);
  servo1->setAngleThrown(100);

  servo2->setAngleClosed(80);
  servo2->setAngleThrown(100);

  // servo2->setTimeFromClosedToThrown_mS(1000);
  // servo2->setTimeFromThrownToClosed_mS(1000);

  // Modify the active and inactive messages which are published by the input1 object.
  // This means that input1 can be controlled by a button which will position a turnout to its thrown position.
  // Another input will be connected to another button which will set the turnout to its closed position.
  //input1->setActiveMessage("THROWN");
  //input1->setInactiveMessage("");
  // input2->setActiveMessage("CLOSED");
  // input2->setInactiveMessage("");

}

void loop() {
  container.loop();
  webSocket.loop();
}

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
