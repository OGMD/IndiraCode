/* #include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include "DHT.h"

WiFiMulti WiFiMulti;
SocketIOclient socketIO;

// Network constants
#define SSID "Megacable_2.4G_A0BC"
#define WIFI_PASSWORD "akHAHDRk"
#define BACKEND_HOST "api.ittsoft.xyz"
#define BACKEND_PORT 443
#define SSL_ENABLED true

// Device settings
#define DEVICE_UUID "ab91fb0b-a861-4422-9083-3c869468428c"
#define USE_SERIAL Serial
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Actuators
#define MAIN_LIGHT_PIN 23

// Socket events
#define UPDATE_SENSORS_EVENT "update_sensors"
#define SET_ACTUATOR_EVENT "set_actuator"

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        USE_SERIAL.printf("[IOc] Disconnected!\n");
        break;
    case sIOtype_CONNECT:
        USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

        // join default namespace (no auto join in Socket.IO V3)
        socketIO.send(sIOtype_CONNECT, "/");
        break;
    case sIOtype_EVENT:
    {
        char *sptr = NULL;
        int id = strtol((char *)payload, &sptr, 10);
        // USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
        if (id)
        {
            payload = (uint8_t *)sptr;
        }
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error)
        {
            USE_SERIAL.print(F("deserializeJson() failed: "));
            USE_SERIAL.println(error.c_str());
            return;
        }

        String eventName = doc[0];
        
        if (eventName == SET_ACTUATOR_EVENT) {
            String actuator = doc[1]["actuator"];
            String actuatorPayload = doc[1];
            String strActuatorState = doc[1]["state"];
            int actuatorState = strActuatorState == "true" ? 1 : 0;
            USE_SERIAL.println(strActuatorState);   
            USE_SERIAL.println(actuatorState);   

            // USE_SERIAL.printf("[IOc] My event name: %s\n", eventName);
            if (actuator == "mainLight") {
                digitalWrite(MAIN_LIGHT_PIN, actuatorState);
            }
            USE_SERIAL.println(eventName);   
            USE_SERIAL.println(actuatorPayload);   
        }

        // Message Includes a ID for a ACK (callback)
        if (id)
        {
            // creat JSON message for Socket.IO (ack)
            DynamicJsonDocument docOut(1024);
            JsonArray array = docOut.to<JsonArray>();

            // add payload (parameters) for the ack (callback function)
            JsonObject param1 = array.createNestedObject();
            param1["now"] = millis();

            // JSON to String (serializion)
            String output;
            output += id;
            serializeJson(docOut, output);

            // Send event
            socketIO.send(sIOtype_ACK, output);
        }
    }
    break;
    case sIOtype_ACK:
        USE_SERIAL.printf("[IOc] get ack: %u\n", length);
        break;
    case sIOtype_ERROR:
        USE_SERIAL.printf("[IOc] get error: %u\n", length);
        break;
    case sIOtype_BINARY_EVENT:
        USE_SERIAL.printf("[IOc] get binary: %u\n", length);
        break;
    case sIOtype_BINARY_ACK:
        USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
        break;
    }
}

void readAndSendSensorsData()
{
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // creat JSON message for Socket.IO (event)
    DynamicJsonDocument sensorsDoc(1024);
    JsonArray sensorsArray = sensorsDoc.to<JsonArray>();

    // add evnet name
    // Hint: socket.on('event_name', ....
    sensorsArray.add(UPDATE_SENSORS_EVENT);

    // add payload (parameters) for the event
    JsonObject sensorsData = sensorsArray.createNestedObject();
    sensorsData["uuid"] = DEVICE_UUID;
    sensorsData["ph"] = 0;
    sensorsData["temperature"] = temperature;
    sensorsData["humidity"] = humidity;
    sensorsData["brightness"] = 0;
    sensorsData["timestamp"] = "";
    sensorsData["network"] = SSID;
    sensorsData["host"] = BACKEND_HOST;

    // JSON to String (serializion)
    String sensorsOutput;
    serializeJson(sensorsDoc, sensorsOutput);

    // Send event
    socketIO.sendEVENT(sensorsOutput);

    // Print JSON for debugging
    USE_SERIAL.println(sensorsOutput);
}

void setup()
{
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    // Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    // Set actuator outputs
    pinMode(MAIN_LIGHT_PIN, OUTPUT);

    for (uint8_t t = 4; t > 0; t--)
    {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    dht.begin();
    WiFiMulti.addAP(SSID, WIFI_PASSWORD);

    // WiFi.disconnect();
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(100);
    }

    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL
    if (SSL_ENABLED)
    {
        socketIO.beginSSL(BACKEND_HOST, BACKEND_PORT, "/socket.io/?EIO=4");
    }
    else
    {
        USE_SERIAL.println("Uso SSL DISABLED");
        socketIO.begin(BACKEND_HOST, BACKEND_PORT, "/socket.io/?EIO=4");
    }

    // event handler
    socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;
void loop()
{
    socketIO.loop();
    
    uint64_t now = millis();

    if (now - messageTimestamp > 2000)
    {
        messageTimestamp = now;
        readAndSendSensorsData();
    }
} */