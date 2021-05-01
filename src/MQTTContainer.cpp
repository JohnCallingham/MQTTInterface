#include <MQTTContainer.h>
#include <MQTTServo.h>
#include <list>
#include <WebSocketsServer.h>
#include <MQTT_EEPROM.h>

extern WebSocketsServer webSocket;
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;
MQTT_EEPROM mqttEEPROM;

MQTTContainer::MQTTContainer() {
    // Start the web server.
    this->server.on ("/", [&]() {this->server.send(200, "text/html", this->indexWebPage);});
    this->server.on ("/servos", [&]() {this->server.send(200, "text/html", this->servosWebPage);});
    this->server.on ("/outputs", [&]() {this->server.send(200, "text/html", this->outputsWebPage);});
    this->server.on ("/inputs", [&]() {this->server.send(200, "text/html", this->inputsWebPage);});
    this->server.begin();
}

MQTTServo* MQTTContainer::addServo(uint8_t pinNumber, const char* servoTopic, Adafruit_PWMServoDriver* pwm) {
    // Check that this pin number has not already been used.
    if (servoPinAlreadyInUse()) throw std::invalid_argument("Servo already in use");

    MQTTServo* newServo = new MQTTServo(pinNumber, servoTopic, pwm);

    // Add the new servo to the end of the servo list.
    servoList.push_back(newServo);

    // Return the pointer to the new servo.
    return newServo;
}

MQTTOutput* MQTTContainer::addOutput(uint8_t pinNumber, const char* outputTopic) {
    return this->addOutput(pinNumber, outputTopic, (Adafruit_MCP23017*)NULL);
}

MQTTOutput* MQTTContainer::addOutput(uint8_t pinNumber, const char* outputTopic, Adafruit_MCP23017* mcp) {
    MQTTOutput* newOutput = new MQTTOutput(pinNumber, outputTopic, mcp);

    // Add the new relay to the end of the output list.
    outputList.push_back(newOutput);

    // Return the pointer to the output.
    return newOutput;
}

MQTTInput* MQTTContainer::addInput(uint8_t pinNumber, const char* inputTopic) {
    return this->addInput(pinNumber, inputTopic, (Adafruit_MCP23017*)NULL);
}

MQTTInput* MQTTContainer::addInput(uint8_t pinNumber, const char* inputTopic, Adafruit_MCP23017* mcp) {
    MQTTInput* newInput = new MQTTInput(pinNumber, inputTopic, mcp);

    // Add the new sensor to the end of the input list.
    inputList.push_back(newInput);

    // Return the pointer to the new input.
    return newInput;
}

MQTT_RGB_LED* MQTTContainer::addRGB_LED(uint8_t ledNumber, const char* ledTopic, RGB_LED_Controller* rgb) {
    MQTT_RGB_LED* newRGB_LED = new MQTT_RGB_LED(rgb, ledNumber, ledTopic);

    // Add the new sensor to the end of the RGB LED list.
    rgbLEDList.push_back(newRGB_LED);

    // Return the pointer to the new RGB LED.
    return newRGB_LED;
}

void MQTTContainer::loop() {
    if (!initialised) {
        this->connectToMQTT();

        // Send startup message to MQTT broker.
        this->publishStartupMessage();

        this->buildIndexWebPage();
        this->buildServosWebPage();
        this->buildOutputsWebPage();
        this->buildInputsWebPage();

        // Set the callback function for websocket events from the client.
        webSocket.onEvent(std::bind(&MQTTContainer::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

        // If EEPROM has not been initialised, then initialise it.
        if (! mqttEEPROM.initialised()) mqttEEPROM.initialise();

        this->initialised = true;
    }

    // Update the MQTT client. This is needed to prevent the connection from dropping.
    mqttClient.loop();

    // If the MQTT connection is lost then attempt to re-establish it.
    if (!mqttClient.connected()) connectToMQTT();
    
    // Iterate over all MQTTServo, MQTTInput and MQTT_RGB_LED objects in their lists calling their loop() method.
    // Using a range-based for loop.
    for (MQTTServo* servo : servoList) {
		servo->loop();
	}
    for (MQTTInput* input : inputList) {
        input->loop();
    }
    for (MQTT_RGB_LED* rgbLED : rgbLEDList) {
        rgbLED->loop();
    }

    server.handleClient();

    // Check to see if a new client has connected to the web socket.
    // If one has, then update the sensor state values in the browser.
    this->handleNewWebSocketClient();
}

void MQTTContainer::handleNewWebSocketClient() {
    // This is called whenever a new client (i.e. browser) connects to the web socket.
    // We need to update all state values.

   if (webSocket.connectedClients(false) != this->connectedClients) { // Setting ping to true caused continuous repeated pings.
       this->connectedClients = webSocket.connectedClients(true);

       // Update the web page with the current state and angle of all servos.
       for (MQTTServo* servo : this->servoList) {
           servo->updateWebPageState();
           servo->updateWebPageAngle();
       }

       // Update the web page with the current state of all basic relays.
       for (MQTTOutput* output : this->outputList) {
           output->updateWebPage();
       }

       // Update the web page with the current state of all sensors.
       for (MQTTInput* input : this->inputList) {
           input->updateWebPage();
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

            // All outputs to subscribe to their topic.
            for (MQTTOutput* output : outputList) {
                mqttClient.subscribe(output->getOutputTopic());

                Serial.printf("Output subscribed to topic: %s\n", output->getOutputTopic());
            }

            // All RGB LEDs to subscribe to their topic.
            for (MQTT_RGB_LED* rgbLED : rgbLEDList) {
                mqttClient.subscribe(rgbLED->getRGB_LED_Topic());

                Serial.printf("RGB LED subscribed to topic: %s\n", rgbLED->getRGB_LED_Topic());
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

    // Using the topic of the received message, determine if this message is for a servo object.
    for (MQTTServo* servo : servoList) {
        if (strcmp(topic, servo->getTurnoutTopic()) == 0) {
            servo->messageReceived(message);
        }
    }

    // Using the topic of the received message, determine if this message is for an output object.
    for (MQTTOutput* output : outputList) {
        if (strcmp(topic, output->getOutputTopic()) == 0) {
            output->messageReceived(charPayload);
        }
    }

    // Using the topic of the received message, determine if this message is for an RGB LED object.
    for (MQTT_RGB_LED* rgbLED : rgbLEDList) {
        if (strcmp(topic, rgbLED->getRGB_LED_Topic()) == 0) {
            rgbLED->messageReceived(charPayload);
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

    <script>
        var Socket;
        function init() {
            Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            Socket.onmessage = function(event) {
                console.log('message received: ' + event.data);
                switch(event.data[0]) {
                    case 'p':
                        alert('EEPROM cleared');
                        break;
                }
            }
            Socket.onopen = function(event) {console.log('Connection opened');}
            Socket.onerror = function(event) {console.log('Error');}
        }
        function clearEEPROM() {
            console.log('clearEEPROM clicked.');
            Socket.send('p');
            console.log('message sent: p');
        }
    </script>

    </head>

    <body onload='javascript:init()'>

    <br />
    <nav>
    Home |
    <a href="inputs">Inputs</a> |
    <a href="outputs">Outputs</a> |
    <a href="servos">Servos</a>
    </nav>
    <br />

    <table>

    <tr>
    <td>Name</td>
    <td>ID</td>
    <td>IP</td>
    <td>EEPROM</td>
    </tr>

    <tr>

    <td>MQTT Interface</td>
    <td>%ID%</td>
    <td>%IP%</td>
    <td>
        <input type='button' value='Clear' onclick="clearEEPROM()" />
    </td>

    </tr>

    </table>
    </body>
    </html>

    )rawliteral";

    indexWebPage = replaceAll(WebPageHTML);
}

void MQTTContainer::buildOutputsWebPage() {
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
    <nav>
    <a href="/">Home</a> |
    <a href="inputs">Inputs</a> |
    Outputs |
    <a href="servos">Servos</a>
    </nav>
    <br />

    )rawliteral";


    outputsWebPage += WebPageHTML;

    outputsWebPage += "<table>\n";

    outputsWebPage += "<tr>";
    outputsWebPage += "<td>Output Pin</td>";
    outputsWebPage += "<td>Output Topic</td>";
    outputsWebPage += "<td>Output State</td>";
    outputsWebPage += "</tr>\n";
    
    // Add a table row for each basic relay.
    for (MQTTOutput* output : outputList) {
        outputsWebPage += "<tr>";
        outputsWebPage += "<td>";
		outputsWebPage += output->getPinString();
        outputsWebPage += "</td>";
        outputsWebPage += "<td>";
		outputsWebPage += String(output->getOutputTopic());
        outputsWebPage += "</td>";
        outputsWebPage += "<td><div id='s";
        //basicRelaysWebPage += relay->getPinString();
        outputsWebPage += output->getPinID();
        outputsWebPage += "'></div>";
        outputsWebPage += "</td>";
        outputsWebPage += "</tr>\n";
	}

    outputsWebPage += "</table>";
    outputsWebPage += "</body>";
    outputsWebPage += "</html>";
}

void MQTTContainer::buildInputsWebPage() {
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
    <nav>
    <a href="/">Home</a> |
    Inputs |
    <a href="outputs">Outputs</a> |
    <a href="servos">Servos</a>
    </nav>
    <br />

    )rawliteral";


    inputsWebPage += WebPageHTML;

    inputsWebPage += "<table>\n";

    inputsWebPage += "<tr>";
    inputsWebPage += "<td>Input Pin</td>";
    inputsWebPage += "<td>Input Topic</td>";
    inputsWebPage += "<td>Input State</td>";
    inputsWebPage += "</tr>\n";
    
    // Add a table row for each basic relay.
    for (MQTTInput* input : inputList) {
        inputsWebPage += "<tr>";
        inputsWebPage += "<td>";
		inputsWebPage += input->getPinString();
        inputsWebPage += "</td>";
        inputsWebPage += "<td>";
		inputsWebPage += String(input->getInputTopic());
        inputsWebPage += "</td>";
        inputsWebPage += "<td><div id='s";
        // inputsWebPage += sensor->getPinString();
        inputsWebPage += input->getPinID();
        inputsWebPage += "'></div>";
        inputsWebPage += "</td>";
        inputsWebPage += "</tr>\n";
	}

    inputsWebPage += "</table>";
    inputsWebPage += "</body>";
    inputsWebPage += "</html>";
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
            "Outputs": [
                %OUTPUTS%
            ],
            "Inputs": [
                %INPUTS%
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
    s.replace("%OUTPUTS%", getOutputsJSON());
    s.replace("%INPUTS%", getInputsJSON());
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

String MQTTContainer::getOutputsJSON() {
    String retValue = "";

    for (MQTTOutput* output : outputList) {
       retValue += "{\"Pin\": \"";
       retValue += output->getPinNumber();
       retValue += "\", \"Topic\": \"";
       retValue += output->getOutputTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

String MQTTContainer::getInputsJSON() {
    String retValue = "";

    for (MQTTInput* input : inputList) {
       retValue += "{\"Pin\": \"";
       retValue += input->getPinNumber();
       retValue += "\", \"Topic\": \"";
       retValue += input->getInputTopic();
       retValue += "\"},\n";
	}

    // Remove last ",".
    retValue = retValue.substring(0, retValue.length() - 2);

    return retValue;
}

void MQTTContainer::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    // This is the web socket callback function.
    // payload[0] = command. payload[1] = N or X. payload[2-3] = port (00-99). payload[4-] = message.
    char pinString[4];

    if (type == WStype_TEXT) {

        // Determine the servo object.
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
                //Serial.printf("Angle changed for servo pin %s, %i\n", servo->getPinString(), angle);

                // Move the servo when the user moves the slider.
                servo->updatePin(angle);

                // Set the current angle to match the slider position. !!wwHAT HAPPENS IF WE ARE PAST THE CLOSED OR THROWN POSITIONS ??? This doesn't seem to be a problem.
                servo->setCurrentAngle(angle);

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
                mqttEEPROM.setServoAngleThrown(servo->getPinNumber(), angle);

                break;
            case 'c':
                // Set Closed has been clicked.
                Serial.printf("Set Closed clicked for servo pin %s, %i\n", servo->getPinString(), angle);
                servo->setAngleClosed(angle);
                mqttEEPROM.setServoAngleClosed(servo->getPinNumber(), angle);

                break;
            case 'p':
                // Clear EEPROM has been clicked.
                Serial.println("Clear EEPROM clicked.");
                mqttEEPROM.initialise();

                // Re-initialise all servos.
                for (MQTTServo* servo : servoList) {
                    servo->initialised = false;
                }

                // Confirm to the web page.
                webSocket.broadcastTXT("p");

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
                            <!-- update the angle figures here -->
                            document.getElementById('t' + event.data.slice(0,4)).innerHTML = event.data.slice(4);
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
            document.getElementById('t' + id).innerHTML = document.getElementById(id).value;
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
    <nav>
    <a href="/">Home</a> |
    <a href="inputs">Inputs</a> |
    <a href="outputs">Outputs</a> |
    Servos
    </nav>
    <br />

    <table>

    <tr>
    <td>Servo Pin</td>
    <td>Servo Topic</td>
    <td>Servo Test</td>
    <td width='200'>Servo State</td>
    <td colspan=2>Servo Angle</td>
    <td>Set Servo Angle</td>
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
        <td width='25'>
            <div align='right' id='tr%PIN_NUMBER%'></div>
        </td>
        <td>
            <div><input type='range' min='0' max='180' id='r%PIN_NUMBER%' oninput='sendAngle(this.id)' /></div>
        </td>
        
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

bool MQTTContainer::servoPinAlreadyInUse() {
return false;
}
