#include <Arduino.h>

enum TRAFFIC_LIGHT_STATE { RED = 0, GREEN = 1, ORANGE= 2 };

class TrafficLight {

    public:
        TrafficLight(int pinRedLight, int pinOrangeLight, int pinGreenLight);
        void setState(TRAFFIC_LIGHT_STATE state);
};