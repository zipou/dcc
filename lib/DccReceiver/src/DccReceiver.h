#include <Arduino.h>

typedef void (*DccReceiverAccessoryCallback)(const bool active, const int group);

class DccReceiver {

    public:
        DccReceiver(int pinData);
        void registerAccessory(int address, DccReceiverAccessoryCallback callback);
};