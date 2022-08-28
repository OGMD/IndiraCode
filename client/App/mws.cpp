#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketClient.h>

const char* ssid = "Megcable_2.4G_557A";
const char* password = "hcuTRcHX";
 
char path[] = "/";
char host[] = "192.168.1.7";
 
WebSocketClient webSocketClient;
WiFiClient client;
 
void connect(){
  if (client.connect(host, 4000)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
  }
 
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
  }
}

void setup() {
  Serial.begin(9600);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  delay(5000);
  connect();
}
 
void loop() {
  String data;
 
  if (client.connected()) {
    webSocketClient.sendData("Info to be echoed back");
    webSocketClient.getData(data);
    if (data.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(data);
    }
  } else {
    Serial.println("Client disconnected.");
    connect();
  }
 
  delay(3000);
}