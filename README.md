# MQTTInterface
A library to interface between JMRI and sensors, servos and relays. Designed to run on a Wemos D1 Mini.
## Functions
* Provides three types of software objects - servos, sensors, and relays - which equate to the JMRI tables - Turnouts, Sensors and Lights.
* A web server is included which displays a multipage interface for configuration and testing.
## Library Dependants
* Nick O'Leary's [PubSubClient libary](https://github.com/knolleary/pubsubclient/) is required to handle the MQTT communications.
* Markus Sattler's [WebSockets library](https://github.com/Links2004/arduinoWebSockets) is required to support the communication between server and client.
## Configurable Items
### MQTTContainer
* setBroker
* setPort
* setStartupTopic
### MQTTRelay
### MQTTSensor
* setDebounceDelay_mS
### MQTTServo
* Creating a servo object. This can be done in two ways.
    - If the servo is connected to a native 8266 port use 'container.addServo(4, "trains/track/turnout/124");'
    - If the servo is connected to an I2C PWM expander port use 'container.addServo(5, "trains/track/turnout/123", pwm);'
* setAngleClosed
* setAngleThrown
* setTimeFromClosedToThrown_mS
* setTimeFromThrownToClosed_mS
* setThrownSensorTopic
* setClosedSensorTopic
