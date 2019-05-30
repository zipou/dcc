#include <Arduino.h>

class DccEmitter {

    public:
        DccEmitter(int pinOne, int pinTwo);
        void updateGroup(int address, bool f0, bool f1, bool f2, bool f3, bool f4, bool f5, bool f6, bool f7, bool f8, bool f9, bool f10, bool f11, bool f12);
        void updateSpeed(int address, int speed, bool isReverse);
        void updateAccessory(int address, int group, bool isActive);
        void loop();
};