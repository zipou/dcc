#include "MqttLib.h"

#include <ArduinoJson.h>

WiFiClientSecure _wificlient;
MQTTClient _client;

MqttLibCallback MqttLib::_callback;
MqttLibErrorCallback MqttLib::_errorCallback;
MqttLibConnectCallback MqttLib::_connectCallback;

char* _username;
char* _password;
const char* _clientId;
char* _topicList;

void MqttLib::setErrorCallback(MqttLibErrorCallback callback) {
  MqttLib::_errorCallback = callback;
}

void MqttLib::setOnConnectCallback(MqttLibConnectCallback callback) {
  MqttLib::_connectCallback = callback;
}

void MqttLib::setOnMessageCallback(MqttLibCallback callback) {
  MqttLib::_callback = callback;
  struct call {
    static void a(String &topic, String &payload) {
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.println("Error during deserialization");
      }
      const char* id = doc["id"];
      Serial.println("Message Recived Through MQTT with id");
      Serial.println(id);

       //Acknowledging
      // ArduinoJson 6
      // DynamicJsonDocument jsonAcknowledge(1024);
      // jsonAcknowledge["type"] = "acknowledge";
      // jsonAcknowledge["id"] = id;
      // char charBuffer[250];
      // serializeJson(doc, charBuffer);
      // // _client.publish("esp_ack", charBuffer);

      if (MqttLib::_callback != NULL) {
        (*MqttLib::_callback)(topic.c_str(), payload.c_str());
      }
    }
  };
  _client.onMessage(call::a);

}

// void MqttLib::acknowledge(String payload) {
//   // StaticJsonBuffer<200> jsonBuffer;
//   // JsonObject& root = jsonBuffer.parseObject(payload);
//   // const char* id = root["id"];
//   // Serial.println("Message Recived Through MQTT with id");
//   // Serial.println(id);
// }

void MqttLib::init(char* host, int port, char* username, char* password, const char* clientId) {
  _client.begin(host, port, _wificlient);
  // _client.setTimeout(500);
  _client.setOptions(2, true, 500);
  _username= username;
  _password= password;
  _clientId= clientId;
  MqttLib::connect();
}

void MqttLib::publish(const char* topic, const char* message) {
  if (! _client.connected()) {
    if (! MqttLib::connect()) {
        return;
    }
  }
  _client.publish(topic, message);
}



bool MqttLib::connect() {
  int i = 0;
  Serial.print("Trying to connect to MQTT");
  // while (true) {
  while (!_client.connect(_clientId, _username, _password)) {
    Serial.print(".");
    delay(1000);
    if (i >= 10) {
      if (MqttLib::_errorCallback != NULL) {
        (*MqttLib::_errorCallback)();
      }
      return false;
    }
    ++i;
  }
  Serial.println("MQTT connected");
  if (MqttLib::_connectCallback != NULL) {
    (*MqttLib::_connectCallback)();
  }
  return true;
}

void MqttLib::subscribe(const char *topic) {
  Serial.println("Subscribed to");
  Serial.println(topic);
  _client.subscribe(topic);
}

void MqttLib::loop() {
  _client.loop();
}

MqttLib::MqttLib() {

}
