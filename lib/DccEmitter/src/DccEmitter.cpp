#include "DccEmitter.h"


int _pinOne;
int _pinTwo;


void sendMinus18() {
  digitalWrite(_pinOne, LOW);
  digitalWrite(_pinTwo, HIGH);
}

void sendPlus18() {
  digitalWrite(_pinTwo, LOW);
  digitalWrite(_pinOne, HIGH);
}

void bitun() { //genere un bit à 1
  //digitalWrite(sdcc, LOW);   // sets the pin off
  sendMinus18();
  delayMicroseconds(58); // pauses for 58 microseconds
  //digitalWrite(sdcc, HIGH);    // sets the pin on
  sendPlus18();
  delayMicroseconds(58); // pauses for 58 microseconds
}

void bitzero() { //genere un bit à 0
  //  digitalWrite(sdcc, LOW);   // sets the pin off
  sendMinus18();
  delayMicroseconds(116); // pauses for 116 microseconds
  sendPlus18();
  delayMicroseconds(116); // pauses for 116 microsec
}


void sendChecksum(int point1[8], int point2[8]) {
  // Serial.println("Envoi du checksum");
  int check[8];

  for (int i = 0; i <= 7; i++) {
    check[i] = point1[i] ^ point2[i];
  }

  // Envoi du checksum
  for (int i = 7; i >= 0; i = i - 1) {
    if (check[i] == 1) bitun();
    if (check[i] == 0) bitzero();
  }
}

void sendPacket(int address[], int command[]) {
  // Serial.println("Envoi d'un paquet");

  // entete du paquet
  for (int i = 0; i <= 15; i++) {
    bitun();
  }
  bitzero();

  // octet d'address
  for (int i = 7; i >= 0; i = i - 1) {
    if (address[i] == 1) bitun();
    if (address[i] == 0) bitzero();
  }

  // bit à zéro
  bitzero();

  // octet de commande
  for (int i = 7; i >= 0; i = i - 1) {
    if (command[i] == 1) bitun();
    if (command[i] == 0) bitzero();
  }

  // bit à zéro
  bitzero();

  // octet de controle
  // Calcul du checksum
  sendChecksum(address, command);
  bitun();

  // pause émission de 25 zéros
  for (int i = 0; i <= 25; i++) {
    bitzero();
  }
}


void sendIdlePacket() {
  // Serial.println("Envoi d'un paquet idle");

  // entete du paquet
  for (int i = 0; i <= 15; i++) {
    bitun();
  }
  bitzero();

  // octet d'address
  for (int i = 7; i >= 0; i = i - 1) {
    bitun();
  }

  // bit à zéro
  bitzero();

  // octet de commande
  for (int i = 7; i >= 0; i = i - 1) {
    bitzero();
  }

  // bit à zéro
  bitzero();

  // octet de controle
  // Calcul du checksum
  // sendChecksum(address, command);
  // // Envoi du checksum
  for (int i = 7; i >= 0; i = i - 1) {
    bitun();
  }

  // bit à un
  bitun();

  // pause émission de 25 zéros
  for (int i = 0; i <= 25; i++) {
    bitzero();
  }
}


void generateAddress(int id, int* a) {
  // int a[10];

  a[7] = 0;
  a[6] = (int)(id / 64);

  if (a[6] == 1) id = id - 64;
  a[5] = (int)(id / 32);

  if (a[5] == 1) id = id - 32;
  a[4] = (int)(id / 16);

  if (a[4] == 1) id = id - 16;
  a[3] = (int)(id / 8);

  if (a[3] == 1) id = id - 8;
  a[2] = (int)(id / 4);

  if (a[2] == 1) id = id - 4;
  a[1] = (int)(id / 2);

  if (a[1] == 1) id = id - 2;
  a[0] = (int)(id);
}

void generateAccessoryAddress(int address, int* a) {

  a[7] = 1;
  a[6] = 0;

  a[5] = (int)(address / 32);

  if (a[5] == 1) address = address - 32;
  a[4] = (int)(address / 16);

  if (a[4] == 1) address = address - 16;
  a[3] = (int)(address / 8);

  if (a[3] == 1) address = address - 8;
  a[2] = (int)(address / 4);

  if (a[2] == 1) address = address - 4;
  a[1] = (int)(address / 2);

  if (a[1] == 1) address = address - 2;
  a[0] = (int)(address);

}

/**
 * TODO
 * Handle the specific for the accessories address (2nd part in the command byte)
 *
 **/
void DccEmitter::updateAccessory(int address, int group, bool isActive) {
  int sendAddress[8];
  generateAccessoryAddress(address, sendAddress);

  int groupBit3 = 0, groupBit2 = 0, groupBit1 = 0;

  if (group % 2 == 0) groupBit1 = 1;

  if (group > 4) {
    groupBit3 = 1;
  }

  if (group == 3 || group == 4 || group == 7 || group == 8) {
    groupBit2 = 1;
  }

  /**
   * 1AAACDDD
   * AAA  second part of the address
   * C    set accessory active
   * DDD  Group target (1 ~ 8)
   **/
  int command[8] = {1, 1,1,1, isActive, groupBit1, groupBit2, groupBit3 };

  sendPacket(sendAddress, command);

}

/*
Trame DCC
Une séquence de synchronisation constituée d'un minimum de 14 bits à 1 suivie d'un bit à 0
  - un octet d'adresse de loco suivi d'un bit à 0
  - un octet de commande suivi d'un bit à 0
  - un octet de controle suivi d'un bit à 1 qui signale la fin de la transmission du paquet.
*/
void DccEmitter::updateSpeed(int address, int speed, bool isReverse) {
  int sendAddress[8];
  generateAddress(address, sendAddress);

  int c[8];

  c[7] = 0;
  c[6] = 1;
  c[5] = (isReverse) ? 1 : 0;

  c[4] = (int)(speed / 16);
  if (c[4] == 1) speed = speed - 16;
  c[3] = (int)(speed / 8);

  if (c[3] == 1) speed = speed - 8;
  c[2] = (int)(speed / 4);

  if (c[2] == 1) speed = speed - 4;
  c[1] = (int)(speed / 2);

  if (c[1] == 1) speed = speed - 2;
  c[0] = (int)(speed);

  sendPacket(sendAddress, c);
}

void DccEmitter::updateGroup(int address, bool f0, bool f1, bool f2, bool f3, bool f4, bool f5, bool f6, bool f7, bool f8, bool f9, bool f10, bool f11, bool f12) {
  int sendAddress[10];
  generateAddress(address, sendAddress);

  int b[8];
  int d[8];
  int e[8];

  // fonctions du groupe 1

  b[0] = f1 ? 1 : 0;
  b[1] = f2 ? 1 : 0;
  b[2] = f3 ? 1 : 0;
  b[3] = f4 ? 1 : 0;;
  b[4] = f0 ? 1 : 0;;
  b[5] = 0;
  b[6] = 0;
  b[7] = 1;


  sendPacket(sendAddress, b);

  // fonctions du groupe 2
  d[0] = f5 ? 1 : 0;;
  d[1] = f6 ? 1 : 0;;
  d[2] = f7 ? 1 : 0;;
  d[3] = f8 ? 1 : 0;;
  d[4] = 1;
  d[5] = 1;
  d[6] = 0;
  d[7] = 1;

  sendPacket(sendAddress, d);

  // fonctions du groupe 3
  e[0] = f9 ? 1 : 0;;
  e[1] = f10 ? 1 : 0;;
  e[2] = f11 ? 1 : 0;;
  e[3] = f12 ? 1 : 0;;
  e[4] = 0;
  e[5] = 1;
  e[6] = 0;
  e[7] = 1;

  sendPacket(sendAddress, e);

}

void DccEmitter::loop() {
  sendIdlePacket();
}

DccEmitter::DccEmitter(int pinOne, int pinTwo) {
  _pinOne = pinOne;
  _pinTwo = pinTwo;
}
