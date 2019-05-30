#include "DccReceiver.h"

const int BIT_ONE_MICRO = 58;
const int BIT_ZERO_MICRO = 116;

const int PRECISION = 15;
const int FACTOR = 2; // Since we use the FALLING interruption, we need to use to detect 2 times the length of a byte in micros (ONE or ZERO)
const int INTERUPTION_TYPE = FALLING;

const int DCC_HEADER_LENGTH = 15;

unsigned long volatile lastUp = 0;
unsigned long volatile durationTemp = 0;

int headerBufferIndex = 0;
bool isHeaderFull = false;

// Buffer to store a Dcc Packet
// bool dccTempBuffer[45];
int dccTempBufferWriteIndex = 0;

bool dccTempAddressBuffer[8];
int dccTempAddressWriteIndex = 0;

bool dccTempCommandBuffer[8];
int dccTempCommandWriteIndex = 0;

bool dccTempChecksumBuffer[8];
int dccTempChecksumWriteIndex = 0;


DccReceiverAccessoryCallback _accessoryCallback;
int _accessoryAddress;
int _pinData;

// void setup() {
//   Serial.begin(115200);
//   pinMode(PIN_DATA, INPUT_PULLUP);
//   pinMode(LEB_BUILTIN, OUTPUT);
// }

/**
 * Resetting all indexes
 **/
void resetIndexes() {
  dccTempBufferWriteIndex= 0;
  dccTempAddressWriteIndex = 0;
  dccTempCommandWriteIndex = 0;
  dccTempChecksumWriteIndex = 0;
  headerBufferIndex = 0;
  isHeaderFull = false;
}

int decodeAccessoryAddress() {
  bool isAccessory = (dccTempAddressBuffer[0] && !dccTempAddressBuffer[1]);
  if (isAccessory) {
    int temp = 0;
    if (dccTempAddressBuffer[2]) temp += 32;
    if (dccTempAddressBuffer[3]) temp += 16;
    if (dccTempAddressBuffer[4]) temp += 8;
    if (dccTempAddressBuffer[5]) temp += 4;
    if (dccTempAddressBuffer[6]) temp += 2;
    if (dccTempAddressBuffer[7]) temp += 1;

    int factor = 0;
    // This field is the "opposite" (111 => 000)
    if (!dccTempCommandBuffer[1]) factor += 4;
    if (!dccTempCommandBuffer[2]) factor += 2;
    if (!dccTempCommandBuffer[3]) factor += 1;

    return (factor != 0) ? temp * factor : temp;
  }
  return -1;
}

int decodeAccessoryGroup() {
  bool isAccessory = (dccTempAddressBuffer[0] && !dccTempAddressBuffer[1]);
  if (isAccessory) {

    if (dccTempCommandBuffer[7]) {
      // 5 <= groupe <= 8
      if (dccTempCommandBuffer[6]) {
        // 7 <= groupe <= 8
        if (dccTempCommandBuffer[5]) {
          // 8
          return 8;
        } else {
          // 7
          return 7;
        }
      } else {
        // 5 <= groupe <= 6
        if (dccTempCommandBuffer[5]) {
          // 6
          return 6;
        } else {
          // 5
          return 5;
        }
      }
    } else {
      // 1 <= groupe <= 4
      if (dccTempCommandBuffer[6]) {
        // 3 <= groupe <= 4
        if (dccTempCommandBuffer[5]) {
          // 4
          return 4;
        } else {
          // 3
          return 3;
        }
      } else {
        // 1 <= groupe <= 2
        if (dccTempCommandBuffer[5]) {
          // 2
          return 2;
        } else {
          // 1
          return 1;
        }
      }
    }
  }
  return -1;
}

/**
 * Handling bit and trying to decode
 * */
void handleBit(bool bit) {

  // Serial.print(bit);

  // Looking for at least 15 ONE for the header to be full
  if (bit == true && isHeaderFull == false) {
    headerBufferIndex++;
    if (headerBufferIndex == DCC_HEADER_LENGTH) {
      isHeaderFull = true;
    }
    return;
  }

  // We are looking for a ZERO when the header is FUll
  if (isHeaderFull && bit == false && dccTempBufferWriteIndex == 0) {
    // Then we can start decoding
    // dccTempBuffer[dccTempBufferWriteIndex] = bit;
    dccTempBufferWriteIndex++;
    return;
  }

  // Here we have the ADDRESS (8 byte)
  if (dccTempBufferWriteIndex > 0 && dccTempBufferWriteIndex <= 8) {
    // dccTempBuffer[dccTempBufferWriteIndex] = bit;
    dccTempBufferWriteIndex++;

    // Putting the address in a separate buffer
    dccTempAddressBuffer[dccTempAddressWriteIndex] = bit;
    dccTempAddressWriteIndex++;
    return;
  }

  // Here should have one zero (the end of the address)
  if (dccTempBufferWriteIndex == 9) {

    if (bit == false) {
      // dccTempBuffer[dccTempBufferWriteIndex] = bit;
      dccTempBufferWriteIndex++;
      return;
    } else {
      //If we detect a one, its not a dcc packet, reset the dcc buffer
      // Serial.print("Error at the end of the address ");
      resetIndexes();
      return;
    }
  }

  // Here we have the COMMAND (8 byte)
  if (dccTempBufferWriteIndex >= 10 && dccTempBufferWriteIndex < 18 ) {

    // dccTempBuffer[dccTempBufferWriteIndex] = bit;
    dccTempBufferWriteIndex++;

    dccTempCommandBuffer[dccTempCommandWriteIndex] = bit;
    dccTempCommandWriteIndex++;
    return;
  }

  // Here should have one zero (the end of the command)
  if (dccTempBufferWriteIndex == 18 ) {
    // Here should have one zero (the end of the command)
    if (bit == false) {
      // dccTempBuffer[dccTempBufferWriteIndex] = bit;
      dccTempBufferWriteIndex++;
      return;
    } else {
      //If we detect a one, its not a dcc packet, reset the dcc buffer
      // Serial.print("Error  at the end of the command");
      resetIndexes();
      return;
    }
  }

  // Here we have the checksum (8 byte)
  if (dccTempBufferWriteIndex >= 19 && dccTempBufferWriteIndex < 27 ) {

    // dccTempBuffer[dccTempBufferWriteIndex] = bit;
    dccTempBufferWriteIndex++;

    dccTempChecksumBuffer[dccTempChecksumWriteIndex] = bit;
    dccTempChecksumWriteIndex++;
    return;
  }

  // Here should have one ONE (the end of the checksum)
  if (dccTempBufferWriteIndex == 27 ) {
    // Serial.print(" Got a checksum ");
    // for (int i = 0; i < 8; i++) {
    //   Serial.print(dccTempChecksumBuffer[i]);
    // }
    // Serial.println(" ");

    // We can check the checksum
    bool isChecksumValid = true;
    for (int i = 0; i < 8; i++) {
      if (dccTempChecksumBuffer[i] != (dccTempAddressBuffer[i] ^ dccTempCommandBuffer[i])) {
        isChecksumValid = false;
      }
    }

    // Serial.print("Checksum is");
    // Serial.println(isChecksumValid);

    if (!isChecksumValid) {
      // Wrong checksum, resetting
      resetIndexes();
    }

    if (bit == true) {
      // dccTempBuffer[dccTempBufferWriteIndex] = bit;
      dccTempBufferWriteIndex++;
      return;
    } else {
      //If we detect a ZERO, its not a dcc packet, reset the dcc buffer
      // Serial.print("Error at the end of the checksum");
      resetIndexes();
      return;
    }
  }

  // Dcc Decoding is done
  if (dccTempBufferWriteIndex == 28 ) {

    // IF ACCESSORY ???
    bool isAccessory = (dccTempAddressBuffer[0] && !dccTempAddressBuffer[1]);
    // Serial.println("");
    // Serial.print(dccTempAddressBuffer[0]);
    // Serial.println("");
    // Serial.print(dccTempAddressBuffer[1]);
    // Serial.println("");
    // Serial.print(isAccessory);
    // Serial.println("");
    if (isAccessory && _accessoryCallback != NULL) {
      bool active = dccTempCommandBuffer[4];
      int group = decodeAccessoryGroup();
      int address = decodeAccessoryAddress();
      if (address == _accessoryAddress) {
        //Calling callback
        (*_accessoryCallback)(active, group);
      }
    }

    // Serial.print("Address ");
    // for (int i = 0; i < 8; i++) {
    //   Serial.print(dccTempAddressBuffer[i]);
    // }
    // Serial.println("");

    // Serial.print("Command ");
    // for (int i = 0; i < 8; i++) {
    //   Serial.print(dccTempCommandBuffer[i]);
    // }
    // Serial.println("");

    // Serial.println(" End ");
    resetIndexes();
    return;
  }

}

/**
 * Detect if the duration of the last interuption matches a DCC byte
 **/
void checkDuration( ) {
  if (durationTemp == 0) return;
  if (durationTemp > ((BIT_ONE_MICRO * FACTOR) - PRECISION) && durationTemp < ((BIT_ONE_MICRO * FACTOR) + PRECISION)) {
    // storeByteIntoBuffer(true);
    handleBit(true);
  }
  if (durationTemp > ((BIT_ZERO_MICRO * FACTOR) - PRECISION) && durationTemp < ((BIT_ZERO_MICRO * FACTOR) + PRECISION)) {
    // storeByteIntoBuffer(false);
    handleBit(false);
  }
  durationTemp = 0;
}

// void loop() {
//   checkDuration();
// }


/**
 * Interuption to catch the impulse timing
**/
void interupt() {
  checkDuration();
  unsigned long now  = micros();
  if (lastUp == 0) {
    // Serial.println("Doing Nothing, probably first run");
  } else {
    unsigned long duration = now - lastUp;
    durationTemp = duration;
  }
  lastUp = now;
}


void DccReceiver::registerAccessory(int address, DccReceiverAccessoryCallback callback) {
  _accessoryCallback = callback;
  _accessoryAddress = address;
}

DccReceiver::DccReceiver(int pinData) {
  // Serial.print("Attaching Interuption to PIN ");
  // Serial.println(pinData);
  _pinData = pinData;
  pinMode(_pinData, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_pinData), &interupt, INTERUPTION_TYPE);
}