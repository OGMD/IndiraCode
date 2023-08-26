#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

//para sensores analogicos solo se puede usar AL ADC_1
const int pot = 34;
const int pot2 = 35;
const int AnalogSensor1 = 39;
const int nivel1 = 32;
const int nivel2 = 33;
//para sensores digitales se puede usar tanto el ADC_1 como el ADC_2
const int sensorDigital = 27;
//salida MOTOR
const int salidaMotor = 26;
int potValor = 0;
int potValor2 = 0;
int nivel1Value = 0;
int nivel2Value = 0;
int sensorDigitalValue = 0;
int AnalogSensor1Value = 0;
WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial
void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n"); 
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            //USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
            USE_SERIAL.printf("Evento: %s\n",payload);
            if(id) {
                payload = (uint8_t *)sptr;
            }
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                USE_SERIAL.print(F("deserializeJson() failed: "));
                USE_SERIAL.println(error.c_str());
                return;
            }
            String eventName = doc[0];
            String evento = "ButtonMain";
            //USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());
            USE_SERIAL.printf("\nHello event: %s\n", eventName.c_str());
            //Agregar los eventos para control
            if(eventName == evento){
                USE_SERIAL.println("ENCENDER LA MAQUINA ---------");
                digitalWrite(LED_BUILTIN,HIGH);
                digitalWrite(salidaMotor,HIGH);
                delay(1200);
                digitalWrite(LED_BUILTIN,LOW);
                digitalWrite(salidaMotor,LOW);
            }
            if(id) {
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

void setup() {
    pinMode(nivel1,INPUT);
    pinMode(nivel2,INPUT);
    pinMode(sensorDigital,INPUT);
    pinMode(LED_BUILTIN,OUTPUT);
    pinMode(salidaMotor,OUTPUT);
    USE_SERIAL.begin(9600);
    USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }
    WiFiMulti.addAP("Megcable_2.4G_557A", "hcuTRcHX");
    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());
    // server address, port and URL
    socketIO.begin("192.168.1.2", 4000, "/socket.io/?EIO=4");
    // event handler
    socketIO.onEvent(socketIOEvent);
}

void readSens(){
    potValor = analogRead(pot);
    potValor2 = analogRead(pot2);
    nivel1Value = digitalRead(nivel1);
    nivel2Value = digitalRead(nivel2);
    sensorDigitalValue = digitalRead(sensorDigital);
    AnalogSensor1Value = analogRead(AnalogSensor1);
    DynamicJsonDocument sensDoc(1024);
    JsonArray sensArray = sensDoc.to<JsonArray>();

    sensArray.add("sens_Data");

    JsonObject sensData = sensArray.createNestedObject();
    sensData["sens1"] = potValor;
    sensData["sens2"] = potValor2;
    sensData["sens3"] = nivel1Value;
    sensData["sens4"] = nivel2Value;
    sensData["sens5"] = sensorDigitalValue;
    sensData["sens6"] = AnalogSensor1Value;
    String sensOutput;
    serializeJson(sensDoc, sensOutput);

    socketIO.sendEVENT(sensOutput);
    Serial.println(sensOutput);
}

void dataProbe(){
        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();
        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("event_name");

        // add payload (parameters) for the event
        JsonObject param1 = array.createNestedObject();
        param1["now"] = "esp is ready";
        param1["next"] ="Siguiente paramatro";
        param1["host"]= "192.168.1.2";
        // JSON to String (serializion)
        String output;
        serializeJson(doc, output);
        // Send event
        socketIO.sendEVENT(output);
        // Print JSON for debugging
        USE_SERIAL.println(output);
}
unsigned long messageTimestamp = 0;
void loop() {
    socketIO.loop();

    uint64_t now = millis();

    if(now - messageTimestamp > 2000) {
        messageTimestamp = now;

        dataProbe();
        readSens();
    }
}