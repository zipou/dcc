#include "TrafficLight.h"


int _pinRed, _pinOrange, _pinGreen;

TrafficLight::TrafficLight(int pinRed, int pinOrange, int pinGreen) {
  _pinRed = pinRed;
  _pinOrange = pinOrange;
  _pinGreen = pinGreen;
  pinMode(pinRed, OUTPUT);
  pinMode(pinOrange, OUTPUT);
  pinMode(pinGreen, OUTPUT);
}

void TrafficLight::setState(TRAFFIC_LIGHT_STATE state) {
  switch (state) {
    case RED:
      digitalWrite(_pinGreen, LOW);
      digitalWrite(_pinOrange, LOW);
      digitalWrite(_pinRed, HIGH);
      break;
    case ORANGE:
      digitalWrite(_pinGreen, LOW);
      digitalWrite(_pinOrange, HIGH);
      digitalWrite(_pinRed, LOW);
      break;
    case GREEN:
      digitalWrite(_pinGreen, HIGH);
      digitalWrite(_pinOrange, LOW);
      digitalWrite(_pinRed, LOW);
      delay(1500);
      break;
  }
}


