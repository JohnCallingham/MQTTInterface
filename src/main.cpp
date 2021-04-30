// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <Arduino.h>
#include <WiFi.h>
#include <MQTTServo.h>
#include <MQTTContainer.h>
#include <WebSocketsServer.h>
#include <MQTTOutput.h>
#include <MQTTInput.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_MCP23017.h>
#include <RGB_LED_Controller.h>
#include <MQTT_RGB_LED.h>

WiFiClient wifiClient;
PubSubClient mqttClient;
WebSocketsServer webSocket(81);

// Create a pointer to an Adafruit_MCP23017 object. This is the I2C I/O expander which can be used for digital inputs and digital outputs.
Adafruit_MCP23017* mcp = new Adafruit_MCP23017(); // This defaults to the I2C address 0x20.

// Create a pointer to an Adafruit_PWMServoDriver object. This is the I2C PWM expander which will be used for servos.
Adafruit_PWMServoDriver* pwm = new Adafruit_PWMServoDriver(0x40);

// Create a pointer to a RGB_LED_Controller object.
RGB_LED_Controller* rgb = new RGB_LED_Controller(); // NUM_LEDS and DATA_PIN are defined in RGB_LED_Controller.h

// Create an MQTTContainer object which will contain pointers to all the MQTTInput, MQTTOutput, MQTTServo and MQTT_RGB_LED objects.
MQTTContainer container;


void setup() {
  Serial.begin(115200);

  // Initialise the I/O expander.
  mcp->begin();

  // Initialise the PWM expander.
  pwm->begin();
  pwm->setOscillatorFrequency(27000000);
  pwm->setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  container.setStartupTopic("events");

  try {
 
/***
 * Create the input objects.
 ***/
  // Create a pointer to an MQTTInput object which is connected to GPIO14 of the 8266.
  MQTTInput* input1 = container.addInput(14, "trains/track/sensor/GPIO14");
  input1->getPinID(); // Keep the compiler happy!

  // Create a pointer to an MQTTInput object which is connected to port 8 of the I2C I/O expander.
  MQTTInput* input2 = container.addInput(8, "trains/track/sensor/Port8", mcp);
  input2->getPinID(); // Keep the compiler happy!
  
  // Create a pointer to an MQTTInput object whose ACTIVE message will be changed to allow a button to be connected to this input.
  // The button will cause the turnout to move to its thrown position.
  MQTTInput* input3 = container.addInput(6, "trains/track/turnout/123", mcp);
  // Modify the active and inactive messages which are published by the input3 object.
  input3->setActiveMessage("THROWN");
  input3->setInactiveMessage("");

  // Create a pointer to an MQTTInput object whose ACTIVE message will be changed to allow a button to be connected to this input.
  // The button will cause the turnout to move to its closed position.
  MQTTInput* input4 = container.addInput(7, "trains/track/turnout/123", mcp);
  // Modify the active and inactive messages which are published by the input4 object.
  input4->setActiveMessage("CLOSED");
  input4->setInactiveMessage("");

/***
 * Create the output objects.
 ***/
  // Create a pointer to an MQTTOutput object which is connected to an 8266 GPIO pin.
  //  This will actually turn the built in LED on and off for testing.
  MQTTOutput* output1 = container.addOutput(LED_BUILTIN, "trains/track/light/L001");
  output1->getPinID(); // Keep the compiler happy!
  
  // Create a pointer to an MQTTOutput object which represents an output which is connected to port 14 of the I2C I/O expander.
  //MQTTOutput* output2 = container.addOutput(14, "trains/track/light/L002", mcp);
  
/***
 * Create the servo objects.
 ***/
  // Create a pointer to an MQTTServo object which is connected port 0 of the I2C PWM expander.
  MQTTServo* servo1 = container.addServo(0, "trains/track/turnout/123", pwm);
  servo1->setClosedTopic("trains/track/sensor/456");
  servo1->setThrownTopic("trains/track/sensor/789");
  servo1->setMidPointTopic("trains/track/light/L001");

  // Create a pointer to an MQTTServo object which is connected to port 15 of the I2C PWM expander.
  MQTTServo* servo2 = container.addServo(15, "trains/track/turnout/124", pwm);
  servo2->setTimeFromClosedToThrown_mS(1000);
  servo2->setTimeFromThrownToClosed_mS(1000);

/***
 * Create the RGB LED objects.
 ***/
  // Create a pointer to an MQTT_RGB_LED object.
  // This will control the first LED in the string which defaults to White when an "ON" message is received and Black when an "OFF" message is received.
  MQTT_RGB_LED* led1_WHITE = container.addRGB_LED(rgb, 0, "trains/track/light/L002");
  led1_WHITE->setOnTime(500);
  led1_WHITE->setOffTime(500);
  
  // Create a pointer to another MQTT_RGB_LED object.
  // This will also control the first LED in the string but will cause a different colour to be displayed.
  // The defaults are overridden below.
  MQTT_RGB_LED* led1_RED = container.addRGB_LED(rgb, 0, "trains/track/light/LED1_Red");
  led1_RED->setOnColour(CRGB::Red);
  led1_RED->setOffColour(CRGB::Black);

  // Create a pointer to another MQTT_RGB_LED object.
  // This will also control the first LED in the string but will cause a different colour to be displayed.
  MQTT_RGB_LED* led1_GREEN = container.addRGB_LED(rgb, 0, "trains/track/light/LED1_Green");
  led1_GREEN->setOnColour(CRGB::Green);
  led1_GREEN->setOffColour(CRGB::Black);

  // Create a pointer to another MQTT_RGB_LED object.
  // This will also control the first LED in the string but will cause a different colour to be displayed.
  MQTT_RGB_LED* led1_BLUE = container.addRGB_LED(rgb, 0, "trains/track/light/LED1_Blue");
  led1_BLUE->setOnColour(CRGB::Blue);
  led1_BLUE->setOffColour(CRGB::Black);

  
  } catch (const std::invalid_argument& e) {
    //std::cerr << e.what() << '\n';
    // Configuration error found so blink the builtin LED.
    Serial.println("Error found");

    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {


  digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level)
  delay(1000); // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  delay(1000); // Wait for two seconds

//above code continually crashing ???

    }
  }

  // No error so continue with the setup.
  connectToWiFi();
  webSocket.begin();
}

void loop() {
  container.loop();
  webSocket.loop();
}
