# MQTTInterface
A program to interface between JMRI and digital inputs, digital outputs and servos. Designed to run on a Wemos D1 Mini.
## Functions
* Provides three types of software objects - servos, inputs, and outputs - which equate to the JMRI tables - Turnouts, Sensors and Lights.
* A web server is included which displays a multipage interface for configuration and testing.
## Library Dependants
* Nick O'Leary's [PubSubClient libary](https://github.com/knolleary/pubsubclient/) is required to handle the MQTT communications.
* Markus Sattler's [WebSockets library](https://github.com/Links2004/arduinoWebSockets) is required to support the communication between web server and client.
* Adafruit's [PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library) is required to interface to the PCA9685 servo driver IC.
* Adafruit's [MCP23017 Arduino Library](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library) is required to interface to the MCP23107 digital input/output IC.
## Software Items
### MQTTContainer
This single object contains MQTTInput, MQTTOutput and MQTTServo objects. The following methods are available for configuration;-
* setBroker(). This allows the name of the MQTT Broker to be set. Defaults to "raspberrypi".
* setPort(). This allows the port of the MQTT Broker to be set. Defaults to 1883.
* setStartupTopic(). This allows the startup topic to be set. Defaults to "events". This is the MQTT topic used to announce that the program has started and the message published to this topic gives information about the program's configuration.
### MQTTInput
Objects of this class can be added to the container by the use of the container's addInput() method. The parameters to this method are;-
* pinNumber. This is the port number on the MCP23017 or the GPIO port on the 8266.
* inputTopic. This is the MQTT topic to which messages are published for this input object.
* mcp. Optional. If specified then pinNumber refers to the port on the MCP23017. If not, then it is a GPIO port on the 8266.

The following methods are available for configuration;-
* setDebounceDelay_mS(). This allows the debouce delay in milli seconds to be specified. Defaults to 50 mS.
* setActiveMessage(). This allows the MQTT message sent when the input becomes active to be set. Defaults to "ACTIVE".
* setInactiveMessage(). This allows the MQTT message sent when the input becomes inactive to be set. Defaults to "INACTIVE".
### MQTTOutput
Objects of this class can be added to the container by the use of the container's addOutput() method. The parameters to this method are;-
* pinNumber. This is the port number on the MCP23017 or the GPIO port on the 8266.
* outputTopic. This is the MQTT topic to which the output object subscribes.
* mcp. Optional. If specified then pinNumber refers to the port on the MCP23017. If not, then it is a GPIO port on the 8266.

There are no methods available for configuration.
### MQTTServo
Objects of this class can be added to the container by the use of the container's addServo() method. The parameters to this method are;-
* pinNumber. This is the port number on the PWM servo driver.
* servoTopic. This is the MQTT topic to which the servo object subscribes.
* pwm. This is the PWM servo driver.

The following methods are available for configuration;-
* setTimeFromClosedToThrown_mS. This time is the same regardless of how far the servo has to move.
* setTimeFromThrownToClosed_mS. This time is the same regardless of how far the servo has to move.
* setThrownTopic. If this value is set the software will publish an MQTT message to this topic when the servo reaches the thrown angle.
* setClosedTopic. If this value is set the software will publish an MQTT message to this topic when the servo reaches the closed angle.
* setMidPointTopic. If this value is set the software will publish an MQTT message to this topic when the servo reaches the mid point angle.

### MQTT_RGB_LED
Objects of this class can be added to the container by the user of the container's addRGB_LED() method. The parameters to this method are;-
* ledNumber. The zero based number of the LED in the string of individually addressable RGB LEDs.
* ledTopic. This is the MQTT topic to which the RGB LED object subscribes.
* rgb. This is the RGB LED controller object.

The following are examples of use;-
*       MQTT_RGB_LED* led1_WHITE = container.addRGB_LED(0, "trains/track/light/LED1", rgb);
This will create the default LED which will display white when an ON message is received and black when an OFF message is received.
*       MQTT_RGB_LED* led1_RED = container.addRGB_LED(0, "trains/track/light/LED1_Red", rgb);
        led1_RED->setOnColour(CRGB::Red);
        led1_RED->setOffColour(CRGB::Black);
This will create an LED which will display red when an ON mesasge is received and black when an OFF message is received.
*       MQTT_RGB_LED* led1_WHITE = container.addRGB_LED(0, "trains/track/light/L002", rgb);
        led1_WHITE->setOnTime(500);
        led1_WHITE->setOffTime(500);
This will create an LED which will blink showing the default on colour (white) for 500mS and then the default off colour (black) for 500mS when an ON message is received. The LED will show black when an OFF message is received.
