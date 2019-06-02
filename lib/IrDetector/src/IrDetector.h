#include <Arduino.h>

class IrDetector {

    public:
      IrDetector(int pinData);
      IrDetector(int pinData, int thresold);
};