#include <MQTTContainer.h>
#include <MQTTServo.h>
#include <list>
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;

MQTTContainer::MQTTContainer() {
    // Start the web server.
    this->server.on ("/", [&]() {this->server.send(200, "text/html", this->indexWebPage);});
    this->server.on ("/servos", [&]() {this->server.send(200, "text/html", this->servosWebPage);});
    this->server.on ("/basicRelays", [&]() {this->server.send(200, "text/html", this->basicRelaysWebPage);});
    this->server.on ("/advancedRelays", [&]() {this->server.send(200, "text/html", this->advancedRelaysWebPage);});
    this->server.on ("/sensors", [&]() {this->server.send(200, "text/html", this->sensorsWebPage);});
    this->server.begin();
}

MQTTServo* MQTTContainer::addServo(uint8_t pinNumber, const char* servoTopic) {
    return this->addServo(pinNumber, servoTopic, (Adafruit_PWMServoDriver*)NULL);
}

MQTTServo* MQTTContainer::addServo(uint8_t pinNumber, const char* servoTopic, Adafruit_PWMServoDriver* pwm) {
    MQTTServo* newServo = new MQTTServo(pinNumber, servoTopic, pwm);

    // Add the new servo to the end of the servo list.
    servoList.push_back(newServo);

    // Return the pointer to the new servo.
    return newServo;
}

MQTTRelay* MQTTContainer::addRelay(uint8_t pinNumber, const char* relayTopic) {
    return this->addRelay(pinNumber, relayTopic, (PCF8575*)NULL);
}

MQTTRelay* MQTTContainer::addRelay(uint8_t pinNumber, const char* relayTopic, PCF8575* pcf8575) {
    MQTTRelay* newRelay = new MQTTRelay(pinNumber, relayTopic, pcf8575);

    // Add the new relay to the end of the basic relay list.
    relayBasicList.push_back(newRelay);

    // Return the pointer to the new basic relay.
    return newRelay;
}

MQTTRelay* MQTTContainer::addRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic) {
    return this->addRelay(pinNumber, relayOperateTopic, relayReleaseTopic, (PCF8575*)NULL);
}

MQTTRelay* MQTTContainer::addRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic, PCF8575* pcf8575) {
    MQTTRelay* newRelay = new MQTTRelay(pinNumber, relayOperateTopic, relayReleaseTopic, pcf8575);

    // Add the new relay to the end of the advanced relay list.
    relayAdvancedList.push_back(newRelay);

    // Return the pointer to the new advanced relay.
    return newRelay;
}

MQTTSensor* MQTTContainer::addSensor(uint8_t pinNumber, const char* sensorTopic) {
    return this->addSensor(pinNumber, sensorTopic, (PCF8575*)NULL);
}

MQTTSensor* MQTTContainer::addSensor(uint8_t pinNumber, const char* sensorTopic, PCF8575* pcf8575) {
    MQTTSensor* newSensor = new MQTTSensor(pinNumber, sensorTopic, pcf8575);

    // Add the new sensor to the end of the sensor list.
    sensorList.push_back(newSensor);

    // Return the pointer to the new sensor.
    return newSensor;
}

void MQTTContainer::loop() {
    if (!initialised) {
        this->connectToMQTT();

        // Send startup message to MQTT broker.
        this->publishStartupMessage();

        this->buildIndexWebPage();
        this->buildServosWebPage();
        this->buildBasicRelaysWebPage();
        this->buildAdvancedRelaysWebPage();
        this->buildSensorsWebPage();

        // Set the callback function for websocket events from the client.
        webSocket.onEvent(std::bind(&MQTTContainer::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

        this->initialised = true;
    }

    // Update the MQTT client. This is needed to prevent the connection from dropping.
    mqttClient.loop();
    
    // Iterate over all pointer to MQTTSensor objects in the sensor list calling their loop() method.
    // Using a range-based for loop.
    for (MQTTServo* servo : servoList) {
		servo->loop();
	}

    // for (MQTTRelay* relay : relayBasicList) {
    //     relay->loop(); // REQUIRED???
    // }

    // for (MQTTRelay* relay : relayAdvancedList) {
    //     relay->loop(); // REQUIRED???
    // }

    for (MQTTSensor* sensor : sensorList) {
        sensor->loop();
    }

    server.handleClient();

    // Check to see if a new client has connected to the web socket.
    // If one has, then update the sensor state values in the browser.
    this ->handleNewWebSocketClient();
}

void MQTTContainer::handleNewWebSocketClient() {
    // This is called whenever a new client (i.e. browser) connects to the web socket.
    // We need to update all state values.

   if (webSocket.connectedClients(false) != this->connectedClients) { // Setting ping to true caused continuous repeated pings.
       this->connectedClients = webSocket.connectedClients(true);

       // Update the web page with the current state of all servos.
       for (MQTTServo* servo : this->servoList) {
           servo->updateWebPageState();
       }

       // Update the web page with the current state of all basic relays.
       for (MQTTRelay* relay : this->relayBasicList) {
           relay->updateWebPage();
       }

       // Update the web page with the current state of all advanced relays.
       for (MQTTRelay* relay : this->relayAdvancedList) {
           relay->updateWebPage();
       }

       // Update the web page with the current state of all sensors.
       for (MQTTSensor* sensor : this->sensorList) {
           sensor->updateWebPage();
       }
   }
}

void MQTTContainer::connectToMQTT() {
    char mqtt_client_id[20];

    Serial.println ("\nConnecting to MQTT.");

    sprintf(mqtt_client_id, "esp8266-%x", ESP.getChipId());

    Serial.printf ("mqtt_client_id %s\n", mqtt_client_id);

    //Serial.printf("%s\n", wifiClient.localIP().toString().c_str());

    mqttClient.setClient(wifiClient);
    mqttClient.setServer(this->mqttBroker, this->mqttPort);
    mqttClient.setBufferSize(1024); // Increased from the default.

    // Loop until connected.
    while (!mqttClient.connected()) {
        if (mqttClient.connect(mqtt_client_id)) {
            Serial.printf("Connected to MQTT broker: %s\n", mqttBroker);

            // All servos to subscribe to their turnout topic.
            for (MQTTServo* servo : servoList) {
                mqttClient.subscribe(servo->getTurnoutTopic());

                Serial.printf("Servo subscribed to topic: %s\n", servo->getTurnoutTopic());
            }

            // All basic relays to subscribe to their relay topic.
            for (MQTTRelay* relay : relayBasicList) {
                mqttClient.subscribe(relay->getRelayTopic());

                Serial.printf("Basic relay subscribed to topic: %s\n", relay->getRelayTopic());
            }

            // All advanced relays to subscribe to their relay operate topic and relay relaese topic.
            for (MQTTRelay* relay : relayAdvancedList) {
                mqttClient.subscribe(relay->getRelayOperateTopic());
                mqttClient.subscribe(relay->getRelayReleaseTopic());

                Serial.printf("Advanced relay subscribed to operate topic: %s\n", relay->getRelayOperateTopic());
                Serial.printf("Advanced relay subscribed to release topic: %s\n", relay->getRelayReleaseTopic());
            }

            // Set the callback which will be used for all servos and relays.
            mqttClient.setCallback(std::bind(&MQTTContainer::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying.
        delay(5000);
        }
    }
}

void MQTTContainer::callback(char* topic, byte* payload, unsigned int length) {
    // This is the MQTT callback function.
    char charPayload[20];
    unsigned int i;
    MQTTServo::receivedMessageEnum message;

    // Convert payload to char array.
    for (i = 0; i < length; i++) {
        charPayload[i] = (char)payload[i];
    }
    charPayload[i] = '\0';

    Serial.printf("Message received on topic [%s] %s\n", topic, charPayload);

    if (strcmp(charPayload, "THROWN") == 0) {
        message = MQTTServo::receivedMessageEnum::messageThrown;
    }
    if (strcmp(charPayload, "CLOSED") == 0) {
        message = MQTTServo::receivedMessageEnum::messageClosed;
    }

    // Using the topic of the received messaage, determine which servo this message is for.
    for (MQTTServo* servo : servoList) {
        if (strcmp(topic, servo->getTurnoutTopic()) == 0) {
            servo->messageReceived(message);
        }
    }

    // Using the topic of the received messaage, determine which relay this message is for.
    for (MQTTRelay* relay : relayBasicList) {
        if (strcmp(topic, relay->getRelayTopic()) == 0) {
            relay->receivedRelayTopic(charPayload);
        }
    }

    for (MQTTRelay* relay : relayAdvancedList) {
        if (strcmp(topic, relay->getRelayOperateTopic()) == 0) {
            relay->receivedRelayOperateTopic(charPayload);
        }
    }

    for (MQTTRelay* relay : relayAdvancedList) {
        if (strcmp(topic, relay->getRelayReleaseTopic()) == 0) {
            relay->receivedRelayReleaseTopic(charPayload);
        }
    }
}

void MQTTContainer::buildIndexWebPage() {
    // const char WebPageHTML[] = R"rawliteral(
    String WebPageHTML = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
    <title>MQTT Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style type = "text/css"> 
        .textinput { 
        width: 100px;
        background-color: Gold;
        }
        .textreadonly {
        width: 100px;
        background-color: White;
        }
        .texthidden {
        display: none;
        }
        table {
        border: 1px solid blue;
        border-spacing: 10px;
        }
        td {
        vertical-align: middle;
        border-bottom: 1px solid #ddd;
        }
    </style> 
    </head>

    <body>

    <br />
    Home
    <br />
    <br />
    <a href="sensors">Sensors</a>
    <br />
    <a href="basicRelays">Basic Relays</a>
    <br />
    <a href="advancedRelays">Advanced Relays</a>
    <br />
    <a href="servos">Servos</a>
    <br />
    <br />

    <table>

    <tr>
    <td>Name</td>
    <td>ID</td>
    <td>IP</td>
    </tr>

    <tr>
    <td>
    MQTT Interface
    </td>
    <td>
    %ID%
    </td>
    <td>
    %IP%
    </td>
    </tr>

    </table>
    </body>
    </html>

    )rawliteral";

    // WebPageHTML.replace("%ID%", ESP.getChipId());
    // WebPageHTML.replace("%IP%", WiFi.localIP());

    //indexWebPage += WebPageHTML;
    indexWebPage = replaceAll(WebPageHTML);
}

void MQTTContainer::buildBasicRelaysWebPage() {
    const char WebPageHTML[] = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
    <title>MQTT Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style type = "text/css"> 
        .textinput { 
        width: 100px;
        background-color: Gold;
        }
        .textreadonly {
        width: 100px;
        background-color: White;
        }
        .texthidden {
        display: none;
        }
        table {
        border: 1px solid blue;
        border-spacing: 10px;
        }
        td {
        vertical-align: middle;
        border-bottom: 1px solid #ddd;
        }
    </style> 

    <script>
        var Socket;
        function init() {
            Socket = new WebSocket('ws://' + window.location.hostname + ':81/basicRelays');
            Socket.onmessage = function(event) {
                console.log('message received: ' + event.data);
                try {
                    document.getElementById(event.data.slice(0,4)).innerHTML = event.data.slice(4);
                }
                catch (e) {}
            }
            Socket.onopen = function(event) {console.log('Connection opened');}
            Socket.onerror = function(event) {console.log('Error');}
        }
    </script>
    </head>
    <body onload='javascript:init()'>

    <br />
    <a href='/'>Home</a>
    <br />
    <br />
    <a href="sensors">Sensors</a>
    <br />
    Basic Relays
    <br />
    <a href="advancedRelays">Advanced Relays</a>
    <br />
    <a href="servos">Servos</a>
    <br />
    <br />

    )rawliteral";


    basicRelaysWebPage += WebPageHTML;

    basicRelaysWebPage += "<table>\n";

    basicRelaysWebPage += "<tr>";
    basicRelaysWebPage += "<td>Relay Pin</td>";
    basicRelaysWebPage += "<td>Relay Topic</td>";
    basicRelaysWebPage += "<td>Relay State</td>";
    basicRelaysWebPage += "</tr>\n";
    
    // Add a table row for each basic relay.
    for (MQTTRelay* relay : relayBasicList) {
        basicRelaysWebPage += "<tr>";
        basicRelaysWebPage += "<td>";
		basicRelaysWebPage += relay->getPinString();
        basicRelaysWebPage += "</td>";
        basicRelaysWebPage += "<td>";
		basicRelaysWebPage += String(relay->getRelayTopic());
        basicRelaysWebPage += "</td>";
        basicRelaysWebPage += "<td><div id='s";
        //basicRelaysWebPage += relay->getPinString();
        basicRelaysWebPage += relay->getPinID();
        basicRelaysWebPage += "'></div>";
        basicRelaysWebPage += "</td>";
        basicRelaysWebPage += "</tr>\n";
	}

    basicRelaysWebPage += "</table>";
    basicRelaysWebPage += "</body>";
    basicRelaysWebPage += "</html>";
}

void MQTTContainer::buildAdvancedRelaysWebPage() {
    const char WebPageHTML[] = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
    <title>MQTT Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style type = "text/css"> 
        .textinput { 
        width: 100px;
        background-color: Gold;
        }
        .textreadonly {
        width: 100px;
        background-color: White;
        }
        .texthidden {
        display: none;
        }
        table {
        border: 1px solid blue;
        border-spacing: 10px;
        }
        td {
        vertical-align: middle;
        border-bottom: 1px solid #ddd;
        }
    </style> 

    <script>
        var Socket;
        function init() {
            Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            Socket.onmessage = function(event) {
                console.log('message received ' + event.data);
                try {
                    document.getElementById(event.data.slice(0,4)).innerHTML = event.data.slice(4);
                }
                catch (e) {}
            }
            Socket.onopen = function(event) {console.log('Connection opened');}
            Socket.onerror = function(event) {console.log('Error');}
        }
    </script>
    </head>
    <body onload='javascript:init()'>

    <br />
    <a href='/'>Home</a>
    <br />
    <br />
    <a href="sensors">Sensors</a>
    <br />
    <a href="basicRelays">Basic Relays</a>
    <br />
    Advanced Relays
    <br />
    <a href="servos">Servos</a>
    <br />
    <br />

    )rawliteral";


    advancedRelaysWebPage += WebPageHTML;

    // advancedRelaysWebPage += "</head>";
    // advancedRelaysWebPage += "<body onload='javascript:init()'>";

    // advancedRelaysWebPage += "<br />";
    // advancedRelaysWebPage += "<a href='/'>Home</a>";
    // advancedRelaysWebPage += "<br />";
    // advancedRelaysWebPage += "<br />";

    advancedRelaysWebPage += "<table>\n";

    advancedRelaysWebPage += "<tr>";
    advancedRelaysWebPage += "<td>Relay Pin</td>";
    advancedRelaysWebPage += "<td>Relay Operate Topic</td>";
    advancedRelaysWebPage += "<td>Relay Release Topic</td>";
    advancedRelaysWebPage += "<td>Relay State</td>";
    advancedRelaysWebPage += "</tr>\n";
    
    // Add a table row for each basic relay.
    for (MQTTRelay* relay : relayAdvancedList) {
        advancedRelaysWebPage += "<tr>";
        advancedRelaysWebPage += "<td>";
		advancedRelaysWebPage += relay->getPinString();
        advancedRelaysWebPage += "</td>";
        advancedRelaysWebPage += "<td>";
		advancedRelaysWebPage += String(relay->getRelayOperateTopic());
        advancedRelaysWebPage += "</td>";
        advancedRelaysWebPage += "<td>";
		advancedRelaysWebPage += String(relay->getRelayReleaseTopic());
        advancedRelaysWebPage += "</td>";
        advancedRelaysWebPage += "<td><div id='s";
        //advancedRelaysWebPage += relay->getPinString();
        advancedRelaysWebPage += relay->getPinID();
        advancedRelaysWebPage += "'></div>";
        advancedRelaysWebPage += "</td>";
        advancedRelaysWebPage += "</tr>\n";
	}

    advancedRelaysWebPage += "</table>";
    advancedRelaysWebPage += "</body>";
    advancedRelaysWebPage += "</html>";
}

void MQTTContainer::buildSensorsWebPage() {
    const char WebPageHTML[] = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
    <title>MQTT Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style type = "text/css"> 
        .textinput { 
        width: 100px;
        background-color: Gold;
        }
        .textreadonly {
        width: 100px;
        background-color: White;
        }
        .texthidden {
        display: none;
        }
        table {
        border: 1px solid blue;
        border-spacing: 10px;
        }
        td {
        vertical-align: middle;
        border-bottom: 1px solid #ddd;
        }
    </style> 

    <script>
        var Socket;
        function init() {
            Socket = new WebSocket('ws://' + window.location.hostname + ':81/sensors');
            Socket.onmessage = function(event) {
                console.log('message received: ' + event.data);
                try {
                    document.getElementById(event.data.slice(0,4)).innerHTML = event.data.slice(4);
                }
                catch (e) {}
            }
            Socket.onopen = function(event) {console.log('Connection opened');}
            Socket.onerror = function(event) {console.log('Error');}
        }
    </script>
    </head>
    <body onload='javascript:init()'>

    <br />
    <a href='/'>Home</a>
    <br />
    <br />
    Sensors
    <br />
    <a href="basicRelays">Basic Relays</a>
    <br />
    <a href="advancedRelays">Advanced Relays</a>
    <br />
    <a href="servos">Servos</a>
    <br />
    <br />
       

    )rawliteral";


    sensorsWebPage += WebPageHTML;

    sensorsWebPage += "<table>\n";

    sensorsWebPage += "<tr>";
    sensorsWebPage += "<td>Sensor Pin</td>";
    sensorsWebPage += "<td>Sensor Topic</td>";
    sensorsWebPage += "<td>Sensor State</td>";
    sensorsWebPage += "</tr>\n";
    
    // Add a table row for each basic relay.
    for (MQTTSensor* sensor : sensorList) {
        sensorsWebPage += "<tr>";
        sensorsWebPage += "<td>";
		sensorsWebPage += sensor->getPinString();
        sensorsWebPage += "</td>";
        sensorsWebPage += "<td>";
		sensorsWebPage += String(sensor->getSensorTopic());
        sensorsWebPage += "</td>";
        sensorsWebPage += "<td><div id='s";
        // sensorsWebPage += sensor->getPinString();
        sensorsWebPage += sensor->getPinID();
        sensorsWebPage += "'></div>";
        sensorsWebPage += "</td>";
        sensorsWebPage += "</tr>\n";
	}

    sensorsWebPage += "</table>";
    sensorsWebPage += "</body>";
    sensorsWebPage += "</html>";
}

void MQTTContainer::publishStartupMessage() {
    const char startupMessage[] = R"rawliteral(
        {
            "Event Name": "Device startup",
            "Device": {
                "Name": "MQTT Interface",
                "ID": "%ID%",
                "IP": "%IP%"
            },
            "Servos": [
                %SERVOS%
            ],
            "Basic Relays": [
                %BASIC_RELAYS%
            ],
            "Advanced Relays": [
                %ADVANCED_RELAYS%
            ],
            "Sensors": [
                %SENSORS%
            ]
        }
        )rawliteral";

    String message(startupMessage);

    message = replaceAll(message);

    char c[message.length() + 1];
    strcpy(c, message.c_str());

    mqttClient.publish(this->startupTopic, c);
}

String MQTTContainer::replaceAll(String s) {
    char chipID[10];
    sprintf(chipID, "%04x", ESP.getChipId());
    s.replace("%ID%", chipID);
    s.replace("%IP%", WiFi.localIP().toString());
    s.replace("%SERVOS%",  getServosJSON());
    s.replace("%BASIC_RELAYS%", getBasicRelaysJSON());
    s.replace("%ADVANCED_RELAYS%", getAdvancedRelaysJSON());
    s.replace("%SENSORS%", getSensorsJSON());
    return s;
}

String MQTTContainer::getServosJSON() {
    String retValue = "";

    for (MQTTServo* servo : servoList) {
       retValue += "{\"Pin\": \"";
       retValue += servo->getPinNumber();
       retValue += "\", \"Topic\": \"";
       retValue += servo->getTurnoutTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

String MQTTContainer::getBasicRelaysJSON() {
    String retValue = "";

    for (MQTTRelay* relay : relayBasicList) {
       retValue += "{\"Pin\": \"";
       retValue += relay->getPinNumber();
       retValue += "\", \"Topic\": \"";
       retValue += relay->getRelayTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

String MQTTContainer::getAdvancedRelaysJSON() {
    String retValue = "";

    for (MQTTRelay* relay : relayAdvancedList) {
       retValue += "{\"Pin\": \"";
       retValue += relay->getPinNumber();
       retValue += "\", \"Operate topic\": \"";
       retValue += relay->getRelayOperateTopic();
       retValue += "\", \"Release topic\": \"";
       retValue += relay->getRelayReleaseTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

String MQTTContainer::getSensorsJSON() {
    String retValue = "";

    for (MQTTSensor* sensor : sensorList) {
       retValue += "{\"Pin\": \"";
       retValue += sensor->getPinNumber();
       retValue += "\", \"Topic\": \"";
       retValue += sensor->getSensorTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

void MQTTContainer::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    // payload[0] = command. payload[1] = N or X. payload[2-3] = port (00-99). payload[4-] = message.
    char pinString[4];

    if (type == WStype_TEXT) {

        // Determine the servo object.
        // uint8_t pinNumber = (uint8_t)payload[1] - 48; // TO DO need a better way to convert from ascii char to int. Also need to cope with 2 chars.
        // MQTTServo* servo = determineServo(pinNumber);
        for (int i=1; i<4; i++) {
            pinString[i - 1] = (char)payload[i];
        }
        pinString[3] = '\0';

        MQTTServo* servo = determineServo(pinString);

        // Determine the angle.
        uint16_t angle = (uint16_t) strtol ((const char*) &payload[4], NULL, 10); // this gets everything from character 4 onwards.

        switch (payload[0]) {
            case 'r':
                // The angle slider has been moved.
                Serial.printf("Angle changed for servo pin %s, %i\n", servo->getPinString(), angle);

                //TO DO - need to move the servo when the user moves the slider.
                servo->updatePin(angle);

                break;
            case 'e':
                // Test Close has been clicked.
                Serial.printf("Test Close clicked for servo pin %s\n", servo->getPinString());
                servo->messageReceived(MQTTServo::receivedMessageEnum::messageClosed);

                break;
            case 'f':
                // Test Throw has been clicked.
                Serial.printf("Test Throw clicked for servo pin %s\n", servo->getPinString());
                servo->messageReceived(MQTTServo::receivedMessageEnum::messageThrown);

                break;
            case 't':
                // Set Thrown has been clicked.
                Serial.printf("Set Thrown clicked for servo pin %s, %i\n", servo->getPinString(), angle);
                servo->setAngleThrown(angle);

                break;
            case 'c':
                // Set Closed has been clicked.
                Serial.printf("Set Closed clicked for servo pin %s, %i\n", servo->getPinString(), angle);
                servo->setAngleClosed(angle);

                break;
            default:
                Serial.println ("Unknown response from web page.\n");
                break;


      }
//     if (payload[0] == '#') {
//       uint16_t brightness = (uint16_t) strtol ((const char*) &payload[1], NULL, 10); // this gets everything after character 1.
//       brightness = 1024 - brightness;
//     //   analogWrite(pin_led, brightness);
//     //   Serial.printf("brightness=%i\n", brightness);
//     } else {
// //      for (uint16_t i=0; i<length; i++)
// //        Serial.print((char) payload[i]);
// //        Serial.println();
//     }
  }
}

MQTTServo* MQTTContainer::determineServo(char* pinString) {
    for (MQTTServo* servo : servoList) {
        if (strcmp(servo->getPinString(), pinString) == 0) {
            return servo;
        }
    }

    return NULL; // To keep the compiler happy.
}

void MQTTContainer::buildServosWebPage() {
    String WebPageHTML = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
    <title>MQTT Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style type = "text/css"> 
        .textinput { 
        width: 100px;
        background-color: Gold;
        }
        .textreadonly {
        width: 100px;
        background-color: White;
        }
        .texthidden {
        display: none;
        }
        table {
        border: 1px solid blue;
        border-spacing: 10px;
        }
        td {
        vertical-align: middle;
        border-bottom: 1px solid #ddd;
        }
    </style> 

    <script>
        var Socket;
        function init() {
            Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            Socket.onmessage = function(event) {
                console.log('message received: ' + event.data);
                switch(event.data[0]) {
                    case 's':
                        try {
                            document.getElementById(event.data.slice(0,4)).innerHTML = event.data.slice(4);
                        }
                        catch (e) {}
                        break;
                    
                    case 'r':
                        try {
                            document.getElementById(event.data.slice(0,4)).value = event.data.slice(4);
                        }
                        catch (e) {}
                        break;
                }
            }
            Socket.onopen = function(event) {console.log('Connection opened');}
            Socket.onerror = function(event) {console.log('Error');}
        }
        function sendAngle(id) {
            Socket.send(id + document.getElementById(id).value);
            console.log('message sent: ' + id + document.getElementById(id).value);
        }
        function testClose(id) {
            Socket.send(id);
            console.log('message sent: ' + id);
        }
        function testThrow(id) {
            Socket.send(id);
            console.log('message sent: ' + id);
        }
        function setThrown(pinNumber) {
            message = 't' + pinNumber + document.getElementById('r' + pinNumber).value;
            Socket.send(message);
            console.log('message sent: ' + message);
            alert('Pin number ' + pinNumber + ' thrown angle set to ' + document.getElementById('r' + pinNumber).value + ' degrees.');
        }
        function setClosed(pinNumber) {
            message = 'c' + pinNumber + document.getElementById('r' + pinNumber).value;
            Socket.send(message);
            console.log('message sent: ' + message);
            alert('Pin number ' + pinNumber + ' closed angle set to ' + document.getElementById('r' + pinNumber).value + ' degrees.');
        }
    </script>
    </head>
    <body onload='javascript:init()'>

    <br />
    <a href='/'>Home</a>
    <br />
    <br />
    <a href="sensors">Sensors</a>
    <br />
    <a href="basicRelays">Basic Relays</a>
    <br />
    <a href="advancedRelays">Advanced Relays</a>
    <br />
    Servos
    <br />
    <br />

    <table>

    <tr>
    <td>Servo Pin</td>
    <td>Servo Topic</td>
    <td>Servo Test</td>
    <td width='200'>Servo State</td>
    <td>Servo Angle</td>
    <td>Set Angle</td>
    </tr>

    %REPEATING_TEXT%

    </table>
    </body>
    </html>

    )rawliteral";

    servosWebPage = WebPageHTML;
    servosWebPage.replace("%REPEATING_TEXT%", getRepeatingText());
}

String MQTTContainer::getRepeatingText() {
    String s = "";

    String RepeatingHTML = R"rawliteral(
        <tr>
        <td>%PIN_NUMBER%</td>
        <td>%TURNOUT_TOPIC%</td>
        <td>
        <input type='button' id='e%PIN_NUMBER%' value='Close' onclick="testClose(this.id)" />
        <input type='button' id='f%PIN_NUMBER%' value='Throw' onclick="testThrow(this.id)" />
        </td>
        <td><div id='s%PIN_NUMBER%'></div></td>
        <td><div><input type='range' min='0' max='180' id='r%PIN_NUMBER%' oninput='sendAngle(this.id)' /></td>
        <td>
        <input type='button' id='c%PIN_NUMBER%' value='Closed' onclick="setClosed('%PIN_NUMBER%')" />
        <input type='button' id='t%PIN_NUMBER%' value='Thrown' onclick="setThrown('%PIN_NUMBER%')" />
        </td>
        </tr>
    )rawliteral";

    // Add a table row for each servo.
    for (MQTTServo* servo : servoList) {

        String c = RepeatingHTML;

        c.replace("%PIN_NUMBER%", servo->getPinString());
        c.replace("%TURNOUT_TOPIC%", servo->getTurnoutTopic());

        s += c;
	}

    return s;
}
