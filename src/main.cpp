#include "const.h"
#include <Arduino.h>
#include <Ticker.h>

#include <WiFi.h>
#include <WiFiClient.h>

#include <ArduinoJson.h>

#include <MqttLib.h>
MqttLib mqttlib;

#include <WifiLib.h>
WifiLib wifi = WifiLib(WIFISSID, WIFIPASSWD);

#include <DccLib.h>
DccLib dcc = DccLib(DCC_PINONE, DCC_PINTWO);


bool f0 = false;
bool f1 = false;
bool f2 = false;
bool f3 = false;
bool f4 = false;
bool f5 = false;
bool f6 = false;
bool f7 = false;
bool f8 = false;
bool f9 = false;
bool f10 = false;
bool f11 = false;
bool f12 = false;


String getChipId() {
  // char clientId[15];
  uint8_t chipid[6];
  esp_efuse_mac_get_default(chipid);
  String id = "";
  for (int i = 0; i<= 5; i++) {
    // Serial.println("chipid[i]");
    // Serial.println((char) chipid[i]);

    id += String(chipid[i], HEX);
  }
  return id;
}


// int in1 = 5;
// int in2 = 6;

void restartEsp() {
  Serial.println("Restarting ESP");
  ESP.restart();
}

void mqttErrorCallback() {
  Serial.println("Error with MQTT, restarting ESP");
  restartEsp();
}


void mqttOnConnectCallback() {
  String topic = getChipId() + "/dcc/#";
  Serial.println("mqttOnConnectCallback");
  Serial.println(topic);
  char topicChar[25];
  topic.toCharArray(topicChar, 25);
  mqttlib.subscribe(topicChar);
}

void mqttCallback(const char* topic, const char* message) {

  Serial.println("Received Message");

  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.println("Error during deserialization");
  }
  const char* type = doc["type"];
  const char* address = doc["address"];
  const char* speed = doc["speed"];
  const bool reverse = doc["reverse"];

  const bool isTypeSpeed = (strcmp(type, "speed") == 0);
  const bool isTypeFunction = (strcmp(type, "function") == 0);

  if (address && speed && isTypeSpeed) {
    bool realReverse = reverse;
    int realSpeed = atoi(speed);
    int realAddress = atoi(address);

    Serial.println("For address");
    Serial.println(realAddress);
    Serial.println("Updating speed to");
    Serial.println(realSpeed);
    Serial.println("And reverse set to ");
    Serial.println(realReverse);
    dcc.updateSpeed(realAddress, realSpeed, realReverse);
  }

  if (address && isTypeFunction) {
    Serial.println("Received Function Call");

    int realAddress = atoi(address);
    bool value = (bool) doc["value"];
    int index = (int) doc["index"];

    if (index == 0) f0 = value;
    if (index == 1) f1 = value;
    if (index == 2) f2 = value;
    if (index == 3) f3 = value;
    if (index == 4) f4 = value;
    if (index == 5) f5 = value;
    if (index == 6) f6 = value;
    if (index == 7) f7 = value;
    if (index == 8) f8 = value;
    if (index == 9) f9 = value;
    if (index == 10) f10 = value;
    if (index == 11) f11 = value;
    if (index == 12) f12 = value;

    dcc.updateGroup(realAddress, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12);
  }

  if (!isTypeFunction && !isTypeSpeed) {
    Serial.println("Unknown command");
  }

}


void setup() {
  // pinMode(in1, OUTPUT);
  // pinMode(in2, OUTPUT);
  // timer.start();
  // MqttLibCallback mqttCb = &mqttCallback;

  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DCC_PINONE, OUTPUT);
  pinMode(DCC_PINTWO, OUTPUT);
  wifi.connect();
  Serial.println("ClientId");
  Serial.println(getChipId());
  mqttlib.setErrorCallback(mqttErrorCallback);
  mqttlib.setOnConnectCallback(mqttOnConnectCallback);
  mqttlib.setOnMessageCallback(&mqttCallback);
  mqttlib.init(MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, getChipId().c_str());

}

void loop() {
  mqttlib.loop();
  dcc.loop();
  // if (trigger) {
    // Serial.println("Sending");
  //   updateSpeed(myAddress, mySpeed, myReverse);
  //   trigger = false;
  // } else {
  //   sendIdlePacket();
  // }

  // int sendAddress[10];
  // generateAddress(myAddress, sendAddress);
  // sendEmptyPacket(sendAddress);

     // delay(500);
    // updateSpeed(myAddress, mySpeed, reverse);
  // updateGroup(myAddress, false, false, false, false, false, false, false, false, false, false, false, false, false);
}
