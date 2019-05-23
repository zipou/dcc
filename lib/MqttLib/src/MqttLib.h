#include <WiFiClientSecure.h>
#include <MQTTClient.h>

typedef void (*MqttLibCallback)(const char* topic, const char* payload);
typedef void (*MqttLibErrorCallback)();
typedef void (*MqttLibConnectCallback)();

class MqttLib {

   public:
     MqttLib();
     void init(char* host, int  port, char* username, char* password, const char* clientId);
     bool connect();
     void loop();
     void publish(const char* topic, const char* message);
    //  void ack(String &topic, String &payload);
    //  void static acknowledge(String payload);
     void subscribe(const char* topic);
     void setOnMessageCallback(MqttLibCallback _callback);
     void setOnConnectCallback(MqttLibConnectCallback _connectCallback);
     void setErrorCallback(MqttLibErrorCallback _errorCallback);
     static MqttLibCallback _callback;
     static MqttLibErrorCallback _errorCallback;
     static MqttLibConnectCallback _connectCallback;
   protected:
};
