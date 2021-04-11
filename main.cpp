// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTTServo.h>
#include <MQTTContainer.h>
#include <WebSocketsServer.h>
#include <MQTTOutput.h>
#include <PCF8575.h>
#include <Adafruit_PWMServoDriver.h>

const char* ssid = "BT-SCAKPC";
const char* password = "btvxqTve4aPQkr";

WiFiClient wifiClient;
PubSubClient mqttClient;
WebSocketsServer webSocket(81);

// Create a pointer to a PCF8575 object. This is the I2C I/O expander which can be used for sensors and relays.
PCF8575* pcf8575 = new PCF8575(0x20);

// Create a pointer to an Adafruit_PWMServoDriver object. This is the I2C PWM expander which will be used for servos.
Adafruit_PWMServoDriver* pwm = new Adafruit_PWMServoDriver(0x40);

// Create an MQTTContainer object which will contain pointers to all the MQTTServo, MQTTRelay and MQTTSensor objects.
MQTTContainer container;

// Create a pointer to an MQTTInput object which is connected to GPIO14 of the 8266.
MQTTInput* input1 = container.addInput(14, "trains/track/sensor/GPIO14");

// Create a pointer to an MQTTInput object which is connected to port 5 of the I2C I/O expander.
//  MQTTInput* sensor2 = container.addInput(10, "trains/track/sensor/Port5", pcf8575);

// Create a pointer to an MQTTOutput object which is connected to an 8266 GPIO pin.
//  This will actually turn the built in LED on and off for testing.
MQTTOutput* output1 = container.addOutput(LED_BUILTIN, "trains/track/light/L001");

// Create a pointer to an MQTTOutput object which represents an output which is connected to port 14 of the I2C I/O expander.
MQTTOutput* output2 = container.addOutput(14, "trains/track/light/L001", pcf8575);

// Create a pointer to an MQTTServo object which is connected port 0 of the I2C PWM expander.
MQTTServo* servo1 = container.addServo(0, "trains/track/turnout/123", pwm);

// Create a pointer to an MQTTServo object which is connected to port 15 of the I2C PWM expander.
MQTTServo* servo2 = container.addServo(15, "trains/track/turnout/124", pwm);

// Forward declarations.
void connectToWiFi();

void setup() {
  Serial.begin(115200);

  // Initialise the I/O expander.
  pcf8575->begin();

  //pcf8575->digitalWrite(0,0);

  // Initialise the PWM expander.
  pwm->begin();
  pwm->setOscillatorFrequency(27000000);
  pwm->setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  connectToWiFi();

  webSocket.begin();

  container.setStartupTopic("events");

  servo1->setClosedSensorTopic("trains/track/sensor/456");
  servo1->setThrownSensorTopic("trains/track/sensor/789");

  servo1->setAngleClosed(80);
  servo1->setAngleThrown(100);

  servo2->setAngleClosed(80);
  servo2->setAngleThrown(100);

  // servo2->setTimeFromClosedToThrown_mS(1000);
  // servo2->setTimeFromThrownToClosed_mS(1000);

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

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
