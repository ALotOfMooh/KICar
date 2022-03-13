/*
  Rui Santos
  Complete project details at
  https://RandomNerdTutorials.com/esp32-web-server-websocket-sliders/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <analogWrite.h>

#include "SPIFFS.h"

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
// Replace with your network credentials
const char *ssid = "KI Auto";
const char *password = "KI123456";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");
// Create an Event Source on /events
AsyncEventSource events("/events");

// Set LED GPIO
const int trigPin1 = 15;
const int echoPin1 = 2;

const int trigPin2 = 22;
const int echoPin2 = 23;

long duration1;
float distanceCm1;
long duration2;
float distanceCm2;

unsigned long lastUpdate = 0;

int refreshRate = 200;

int motor1Pin1 = 27;
int motor1Pin2 = 14;
int motor1enable = 12;

int motor2Pin1 = 26;
int motor2Pin2 = 25;
int motor2enable = 33;

int input1 = 0;
int input2 = 0;
int output1 = 0;
int output2 = 0;

boolean powerOn = false;
boolean reverseOn = false;

String message = "";
String sliderValue1 = "0";  // weight11
String sliderValue2 = "0";  // weight12
String sliderValue3 = "0";  // weight21
String sliderValue4 = "0";  // weight 22

// Json Variable to Hold Slider Values
JSONVar sliderValues;

// Get Slider Values
String getSliderValues() {
    sliderValues["sliderValue1"] = String(sliderValue1);
    sliderValues["sliderValue2"] = String(sliderValue2);
    sliderValues["sliderValue3"] = String(sliderValue3);
    sliderValues["sliderValue4"] = String(sliderValue4);
    sliderValues["power"] = String(powerOn);
    sliderValues["reverse"] = String(reverseOn);

    String jsonString = JSON.stringify(sliderValues);
    return jsonString;
}

// Initialize SPIFFS
void initFS() {
    if (!SPIFFS.begin()) {
        Serial.println("An error has occurred while mounting SPIFFS");
    } else {
        Serial.println("SPIFFS mounted successfully");
    }
}

// Initialize WiFi
void initWiFi() {
    //// fuer hotspot
    WiFi.softAP(ssid, password);
    Serial.print("Connecting to WiFi ..");
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    ///////////////////////

    // fuer WIFI

    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);

    // while (WiFi.status() != WL_CONNECTED) {
    //     Serial.print('.');
    //     delay(1000);
    // }
    // Serial.println(WiFi.localIP());

    /////////////////////////////
}

void notifyClients(String sliderValues) { ws.textAll(sliderValues); }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT) {
        data[len] = 0;
        message = (char *)data;
        Serial.print("Webserver: ");
        Serial.println(message);
        if (message.indexOf("1s") >= 0) {
            sliderValue1 = message.substring(2);
            Serial.println(getSliderValues());
            notifyClients(getSliderValues());
        }
        if (message.indexOf("2s") >= 0) {
            sliderValue2 = message.substring(2);
            Serial.println(getSliderValues());
            notifyClients(getSliderValues());
        }
        if (message.indexOf("3s") >= 0) {
            sliderValue3 = message.substring(2);
            Serial.println(getSliderValues());
            notifyClients(getSliderValues());
        }
        if (message.indexOf("4s") >= 0) {
            sliderValue4 = message.substring(2);
            // dutyCycle4 = map(sliderValue4.toInt(), 0, 100, 0, 255);
            // Serial.println(dutyCycle4);
            Serial.println(getSliderValues());
            notifyClients(getSliderValues());
        }
        if (strcmp((char *)data, "getValues") == 0) {
            notifyClients(getSliderValues());
        }
        if (message.indexOf("switch_power") >= 0) {
            if (message.substring(12) == "true") {
                Serial.println("power on");  // power on
                powerOn = true;
            } else if (message.substring(12) == "false") {  // power off
                Serial.println("power off");
                powerOn = false;
            }
        }
        if (message.indexOf("switch_reverse") >= 0) {
            Serial.println(message.substring(14));
            if (message.substring(14) == "true") {  // drives revers
                Serial.println("drive in reverse");
                reverseOn = true;
            } else if (message.substring(14) == "false") {  // drives forward
                Serial.println("drive forward");
                reverseOn = false;
            }
        }
    }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n",
                          client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup() {
    Serial.begin(115200);
    pinMode(motor1enable, OUTPUT);
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);

    pinMode(motor2enable, OUTPUT);
    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);

    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);

    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);
    digitalWrite(motor1enable, LOW);
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2enable, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);

    initFS();
    initWiFi();

    initWebSocket();

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.printf(
                "Client reconnected! Last message ID that it got is: %u\n",
                client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);

    /////////
    server.serveStatic("/", SPIFFS, "/");

    // Start server
    server.begin();
}

void loop() {
    if (millis() - lastUpdate > refreshRate) {
        // Clears the trigPin
        digitalWrite(trigPin1, LOW);
        delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(trigPin1, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin1, LOW);
        duration1 = pulseIn(echoPin1, HIGH);
        distanceCm1 = duration1 * SOUND_SPEED / 2;
        // Clears the trigPin
        digitalWrite(trigPin2, LOW);
        delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(trigPin2, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin2, LOW);
        duration2 = pulseIn(echoPin2, HIGH);
        distanceCm2 = duration2 * SOUND_SPEED / 2;
        Serial.print(distanceCm1);
        Serial.print("   ");
        Serial.println(distanceCm2);
        lastUpdate = millis();
        ////
        input1 = distanceCm1;
        input2 = distanceCm2;
        events.send("ping", NULL, millis());

        if (input1 > 80) input1 = 80;
        if (input2 > 80) input2 = 80;

        output1 =
            (sliderValue1.toInt() * input1 + sliderValue3.toInt() * input2) /
            100;
        output2 =
            (sliderValue2.toInt() * input1 + sliderValue4.toInt() * input2) /
            100;

        if (output1 > 80) output1 = 80;
        if (output2 > 80) output2 = 80;

        events.send(String(input1).c_str(), "input1", millis());
        events.send(String(input2).c_str(), "input2", millis());
        events.send(String(output1).c_str(), "output1", millis());
        events.send(String(output2).c_str(), "output2", millis());

        if (reverseOn) {
            digitalWrite(motor1Pin1, LOW);
            digitalWrite(motor1Pin2, HIGH);
            digitalWrite(motor2Pin1, LOW);
            digitalWrite(motor2Pin2, HIGH);
        } else {
            digitalWrite(motor1Pin1, HIGH);
            digitalWrite(motor1Pin2, LOW);
            digitalWrite(motor2Pin1, HIGH);
            digitalWrite(motor2Pin2, LOW);
        }

        if (output1 > 20 && powerOn) {
            analogWrite(motor1enable, map(output1, 0, 80, 160, 250));
        } else {
            analogWrite(motor1enable, 0);
        }
        if (output2 > 20 && powerOn) {
            analogWrite(motor2enable, map(output2, 0, 80, 160, 250));
        } else {
            analogWrite(motor2enable, 0);
        }
    }

    ws.cleanupClients();
}