/*****************************************************************************/
/*
  Arduino library for Atmel SIGFOX module ATA8520.
  This library uses Arduino SPI library.
*/
/*****************************************************************************/

/*
  Copyright (c) 2016 Arduino LLC

  This software is open source software and is not owned by Atmel;
  you can redistribute it and/or modify it under the terms of the GNU
  Lesser General Public License as published by the Free Software Foundation;
  either version 2.1 of the License, or (at your option) any later version.

  You acknowledge that the ArduinoUNO software is distributed to you free of
  charge on an "as is" basis and that it has not been developed to meet your
  specific requirements. It is supplied in the hope that it will be useful but
  WITHOUT ANY WARRANTY that its use will be uninterrupted or error-free.
  All other conditions, warranties or other terms which might have effect
  between the parties or be implied or incorporated into this licence or any
  collateral contract whether by statute or otherwise are hereby excluded,
  including the implied conditions, warranties or other terms as to
  satisfactory quality, fitness for purpose, non-infringement or the use of
  reasonable skill and care.
  See the GNU Lesser General Public License for more details.
*/


#include "SigFox.h"
#include "SPI.h"

const char str0[]  = "OK";
const char str1[]  = "Manufacturer error";
const char str2[]  = "ID or key error";
const char str3[]  = "State machine error";
const char str4[]  = "Frame size error";
const char str5[]  = "Manufacturer send error";
const char str6[]  = "Get voltage/temperature error";
const char str7[]  = "Close issues encountered";
const char str8[]  = "API error indication";
const char str9[]  = "Error getting PN9";
const char str10[]  = "Error getting frequency";
const char str11[]  = "Error building frame";
const char str12[]  = " in delay routine";
const char str13[] = "Timeout interrupt no transmission";

const char * sigstr[14]  =        // SIGFOX message status
{str0, str1, str2, str3, str4, str5, str6, str7, str8, str9, str10, str11, str12, str13};

#define SPICONFIG   SPISettings(300000UL, MSBFIRST, SPI_MODE0)

int SIGFOXClass::begin()
{
  buffer[0] = 0;
  pinMode(NEVENT, INPUT_PULLUP);
  pinMode(RFPWR, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(RFPWR, HIGH);
  pinMode(SS, OUTPUT);
  pinMode(NRES, OUTPUT);
  digitalWrite(NRES, HIGH);
  delay(10);
  digitalWrite(NRES, LOW);
  delay(10);
  digitalWrite(NRES, HIGH);
  SPI_PORT.begin();
  SPI_PORT.setDataMode(SPI_MODE0);
  SPI_PORT.setBitOrder(MSBFIRST);

  unsigned char *buff=getSigVersion();
  if ((buff[0]|buff[1])==0) 
    return false;
  return true;
}

int SIGFOXClass::sendMessage(char mess[])
{
  int len = strlen(mess);
  return sendMessage((unsigned char*)mess, len);
}

int SIGFOXClass::sendMessage(unsigned char mess[], int len)
{
  if (len == 0) return -1;
  getStatus();             //reset NNEVENT

  digitalWrite(SS, LOW);
  delay(1);
  
  SPI_PORT.beginTransaction(SPICONFIG);
  if (len > 12) len = 12;
  int i = 0;
  
  SPI_PORT.transfer(0x07);
  SPI_PORT.transfer(len);
  for (i = 0; i < len; i++) {
    SPI_PORT.transfer((uint8_t)mess[i]);
  }
  SPI_PORT.endTransaction();

  digitalWrite(SS, HIGH);
  delay(5);
  digitalWrite(SS, LOW);

  SPI_PORT.transfer(0x07);
  SPI_PORT.transfer(len);
  for (i = 0; i < len; i++) {
    SPI_PORT.transfer((uint8_t)mess[i]);
  }
  SPI_PORT.endTransaction();
  
  delay(1);
  digitalWrite(SS, HIGH);

  delay(5);
  calibrateCrystal();
  delay(5);

  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x0D);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  int ret = 99;
  for (i = 0; i < 60000; i++)
  {
    if (digitalRead(NEVENT) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    else {
      digitalWrite(LED1, HIGH);
      delay(50);
      digitalWrite(LED1, LOW);
      delay(50);
    }
  }
  if (ret == 99) sig = 13;
  return sig;
}

int SIGFOXClass::calibrateCrystal() {
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x14);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  int ret = 99;
  for (int i = 0; i < 6000; i++)
  {
    if (digitalRead(NEVENT) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    else {
      digitalWrite(LED1, HIGH);
      delay(50);
      digitalWrite(LED1, LOW);
      delay(50);
    }
  }
  if (ret == 99) sig = 13;
  return sig;
}

int SIGFOXClass::getStatusCode(byte type)
{
  switch (type)
  {
    case 0 : return ssm; break;
    case 1 : return atm; break;
    case 2 : return sig; break;
  }
}

char* SIGFOXClass::getStatusAtm()
{
  buffer[0] = '\0';
  byte err = (atm & B0011110) >> 1;
  char pa[10]; pa[0] = '\0';
  if (bitRead(atm, 0) == 1) strcpy(pa, "PA ON"); else strcpy(pa, "PA OFF");
  if (err > 0)
  {
    snprintf((char*)buffer, BLEN, "Error code: %i", err);
    return (char*)buffer;
  }
  if (bitRead(atm, 5) == 1)
  {
    snprintf((char*)buffer, BLEN, "Frame sent");
    return (char*)buffer;
  }
  if (bitRead(atm, 6) == 1)
  {
    snprintf((char*)buffer, BLEN, "%s . System ready", pa);
    return (char*)buffer;
  }
  snprintf((char*)buffer, BLEN, "%s\0", pa);
  return (char*)buffer;
}

char* SIGFOXClass::getStatusSig()
{
  if (sig > 12)
  {
    snprintf((char*)buffer, BLEN, "Controller comm. error: %d", sig);
    return (char*)buffer;
  }
  strncpy((char*)buffer, sigstr[sig], BLEN);

  return (char*)buffer;
}

void SIGFOXClass::getStatus()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x0A);
  SPI_PORT.transfer(0);
  ssm = SPI_PORT.transfer(0);
  atm = SPI_PORT.transfer(0);
  sig = SPI_PORT.transfer(0);
  sig2 = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
}

void SIGFOXClass::getTemperatureInternal()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x13);
  SPI_PORT.transfer(0);
  vhidle = SPI_PORT.transfer(0);
  vlidle = SPI_PORT.transfer(0);
  vhactive = SPI_PORT.transfer(0);
  vlactive = SPI_PORT.transfer(0);
  temperatureL = SPI_PORT.transfer(0);
  temperatureH = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
}

char* SIGFOXClass::readConfig(int* len)
{

  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x1F);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);

  delay(5);

  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x20);
  SPI_PORT.transfer(0);
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  repeat = SPI_PORT.transfer(0);
  configuration = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  buffer[0] = tx_freq;
  buffer[4] = rx_freq;
  buffer[8] = repeat;
  buffer[9] = configuration;
  buffer[10] = 0;
  *len = 10;
  return (char*)buffer;
}
unsigned char* SIGFOXClass::getAtmVersion()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x06);
  SPI_PORT.transfer(0);
  byte mv = SPI_PORT.transfer(0);
  byte lv = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  buffer[0] = mv;
  buffer[1] = lv;
  buffer[2] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getSigVersion()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x06);
  SPI_PORT.transfer(0);
  byte mv = SPI_PORT.transfer(0);
  byte lv = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  buffer[0] = mv;
  buffer[1] = lv;
  buffer[2] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getID()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x12);
  SPI_PORT.transfer(0);
  byte ID3 = SPI_PORT.transfer(0);
  byte ID2 = SPI_PORT.transfer(0);
  byte ID1 = SPI_PORT.transfer(0);
  byte ID0 = SPI_PORT.transfer(0);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  buffer[0] = ID3; buffer[1] = ID2; buffer[2] = ID1; buffer[3] = ID0;
  buffer[4] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getPAC()
{
  int i;
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x0F);
  SPI_PORT.transfer(0);
  for (i = 0; i < 16; i++) {
    buffer[i] = SPI_PORT.transfer(0);
  }
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  buffer[16] = '\0';
  return buffer;
}

void SIGFOXClass::reset()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x01);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
}

void SIGFOXClass::testMode(byte frameL, byte frameH, byte chanL, byte chanH)
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x08);
  SPI_PORT.transfer(frameL);
  SPI_PORT.transfer(frameH);
  SPI_PORT.transfer(chanL);
  SPI_PORT.transfer(chanH);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
}

void SIGFOXClass::setEUMode()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x11);
  SPI_PORT.transfer(0);
  SPI_PORT.transfer(1);
  SPI_PORT.transfer(0x2);
  SPI_PORT.transfer(0xFB);
  //SPI_PORT.transfer(0x3F);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  int ret = 99;
  for (int i = 0; i < 300; i++)
  {
    if (digitalRead(NEVENT) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    else {
      analogWrite(LED1, 200);
      delay(50);
      analogWrite(LED1, 0);
      delay(50);
    }
  }
  if (ret == 99) {
    Serial.println("Failed to set mode");
  }
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x05);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  delay(100);
}
void SIGFOXClass::end()
{
  digitalWrite(SS, LOW);
  delay(1);
  SPI_PORT.beginTransaction(SPICONFIG);
  SPI_PORT.transfer(0x05);
  SPI_PORT.endTransaction();
  delay(1);
  digitalWrite(SS, HIGH);
  pinMode(RFPWR, HIGH);
}

SIGFOXClass SigFox; //singleton