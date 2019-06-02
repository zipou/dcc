#include "IrDetector.h"

int _pinIrData;
int _iRDetectorThresold = 5000;

int _irTempCount = millis();

void irDetectorInterupt() {
  int now = millis();
  if ((now - _irTempCount) >  _iRDetectorThresold) {
    Serial.println("Interupting##########");
    _irTempCount = now;
  }
}

IrDetector::IrDetector(int pinData) {
  _pinIrData = pinData;
  pinMode(pinData, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinData), &irDetectorInterupt, FALLING);
}

IrDetector::IrDetector(int pinData, int thresold) {
  _pinIrData = pinData;
  _iRDetectorThresold = thresold;
  pinMode(pinData, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinData), &irDetectorInterupt, FALLING);
}
