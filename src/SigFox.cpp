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

#define SPICONFIG   SPISettings(100000UL, MSBFIRST, SPI_MODE0)

void SIGFOXClass::debug(bool on) {
  // Enables debug via LED and Serial prints
  // Also disables greedy sleep strategy
  debugging = on;
  if (debugging) {
    pinMode(led_pin, OUTPUT);
  }
}

int SIGFOXClass::begin(bool configured)
{
#ifdef SIGFOX_SPI
  spi_port = SIGFOX_SPI;
  reset_pin = SIGFOX_RES_PIN;
  poweron_pin = SIGFOX_PWRON_PIN;
  interrupt_pin = SIGFOX_EVENT_PIN;
  chip_select_pin = SIGFOX_SS_PIN;
  led_pin = LED_BUILTIN;
#else
  // begin() can only be used on boards with embedded Sigfox module
  if (configured == false) {
    return false;
  }
#endif

  pinMode(interrupt_pin, INPUT_PULLUP);
  pinMode(poweron_pin, OUTPUT);
  digitalWrite(poweron_pin, HIGH);
  pinMode(chip_select_pin, OUTPUT);
  digitalWrite(chip_select_pin, HIGH);
  pinMode(reset_pin, OUTPUT);
  // Power cycle the chip
  digitalWrite(reset_pin, HIGH);
  delay(10);
  digitalWrite(reset_pin, LOW);
  delay(10);
  digitalWrite(reset_pin, HIGH);
  spi_port.begin();
  spi_port.setDataMode(SPI_MODE0);
  spi_port.setBitOrder(MSBFIRST);

  delay(30);

  unsigned char *buff = getSigVersion();
  if (((buff[0] | buff[1]) == 0) || (buff[0] == 0xFF))
    return false;
  return true;
}

int SIGFOXClass::begin(SPIClass& spi, int reset, int poweron, int interrupt, int chip_select, int led)
{
  spi_port = spi;
  reset_pin = reset;
  poweron_pin = poweron;
  interrupt_pin = interrupt;
  chip_select_pin = chip_select;
  led_pin = led;

  return begin(true);
}

int SIGFOXClass::send(char mess[])
{
  int len = strlen(mess);
  return send((unsigned char*)mess, len);
}

int SIGFOXClass::send(unsigned char mess[], int len)
{
  if (len == 0) return -1;
  getStatus();

  digitalWrite(chip_select_pin, LOW);
  delay(1);
  
  spi_port.beginTransaction(SPICONFIG);
  if (len > 12) len = 12;
  int i = 0;
  
  spi_port.transfer(0x07);
  spi_port.transfer(len);
  for (i = 0; i < len; i++) {
    spi_port.transfer((uint8_t)mess[i]);
  }
  spi_port.endTransaction();

  digitalWrite(chip_select_pin, HIGH);

  delay(5);
  calibrateCrystal();
  delay(5);

  digitalWrite(chip_select_pin, LOW);
  delay(1);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x0D);
  spi_port.endTransaction();
  delay(1);
  digitalWrite(chip_select_pin, HIGH);
  int ret = 99;
  for (i = 0; i < 60000; i++)
  {
    if (digitalRead(interrupt_pin) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    else {
      digitalWrite(led_pin, HIGH);
      delay(50);
      digitalWrite(led_pin, LOW);
      delay(50);
    }
  }
  if (ret == 99) sig = 13;
  return sig;
}

int SIGFOXClass::calibrateCrystal() {
  digitalWrite(chip_select_pin, LOW);
  delay(1);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x14);
  spi_port.endTransaction();
  delay(1);
  digitalWrite(chip_select_pin, HIGH);
  int ret = 99;
  for (int i = 0; i < 6000; i++)
  {
    if (digitalRead(interrupt_pin) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    else {
      digitalWrite(led_pin, HIGH);
      delay(50);
      digitalWrite(led_pin, LOW);
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
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x0A);
  spi_port.transfer(0);
  ssm = spi_port.transfer(0);
  atm = spi_port.transfer(0);
  sig = spi_port.transfer(0);
  sig2 = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
}

void SIGFOXClass::getTemperatureInternal()
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x13);
  spi_port.transfer(0);
  vhidle = spi_port.transfer(0);
  vlidle = spi_port.transfer(0);
  vhactive = spi_port.transfer(0);
  vlactive = spi_port.transfer(0);
  temperatureL = spi_port.transfer(0);
  temperatureH = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
}

char* SIGFOXClass::readConfig(int* len)
{

  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x1F);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);

  delay(5);

  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x20);
  spi_port.transfer(0);
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  tx_freq = SPI.transfer(0) | tx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  rx_freq = SPI.transfer(0) | rx_freq << 8;
  repeat = spi_port.transfer(0);
  configuration = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);

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
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x06);
  spi_port.transfer(0);
  byte mv = spi_port.transfer(0);
  byte lv = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
  buffer[0] = mv;
  buffer[1] = lv;
  buffer[2] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getSigVersion()
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x06);
  spi_port.transfer(0);
  byte mv = spi_port.transfer(0);
  byte lv = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
  buffer[0] = mv;
  buffer[1] = lv;
  buffer[2] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getID()
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x12);
  spi_port.transfer(0);
  byte ID3 = spi_port.transfer(0);
  byte ID2 = spi_port.transfer(0);
  byte ID1 = spi_port.transfer(0);
  byte ID0 = spi_port.transfer(0);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
  buffer[0] = ID3; buffer[1] = ID2; buffer[2] = ID1; buffer[3] = ID0;
  buffer[4] = '\0';
  return buffer;
}

unsigned char* SIGFOXClass::getPAC()
{
  int i;
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x0F);
  spi_port.transfer(0);
  for (i = 0; i < 16; i++) {
    buffer[i] = spi_port.transfer(0);
  }
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
  buffer[16] = '\0';
  return buffer;
}

void SIGFOXClass::reset()
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x01);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
}

void SIGFOXClass::testMode(byte frameL, byte frameH, byte chanL, byte chanH)
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x08);
  spi_port.transfer(frameL);
  spi_port.transfer(frameH);
  spi_port.transfer(chanL);
  spi_port.transfer(chanH);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);
}

void SIGFOXClass::setMode(Country EUMode, TxRxMode tx_rx)
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x11);
  spi_port.transfer(0);
  spi_port.transfer(1);
  spi_port.transfer(0x2);
  uint8_t mode = (0x3 << 4) | (1 << 3) | (EUMode << 2) | (tx_rx << 1) | 1;
  spi_port.transfer(mode);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(1);

  int ret = 99;
  for (int i = 0; i < 300; i++)
  {
    if (digitalRead(interrupt_pin) == 0) {
      getStatus();
      ret = getStatusCode(2);
      break;
    }
    delay(10);
  }
  if (ret == 99) {
    Serial.println("Failed to set mode");
  }

  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x05);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  delay(100);
}

void SIGFOXClass::end()
{
  digitalWrite(chip_select_pin, LOW);
  spi_port.beginTransaction(SPICONFIG);
  spi_port.transfer(0x05);
  spi_port.endTransaction();
  digitalWrite(chip_select_pin, HIGH);
  pinMode(poweron_pin, HIGH);
  delay(1);
}

SIGFOXClass SigFox; //singleton