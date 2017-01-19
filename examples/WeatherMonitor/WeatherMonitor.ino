/*
  SigFox Simple Weather Station

  This sketch demonstrates the usage of MKR3000 as a simple weather station.
  It uses
    the onboard temperature sensor
    HTU21D I2C sensor to get humidity
    Bosch BMP180 to get the barometric pressure
    TSL2561 Light Sensor to get luminosity

  Download the needed libraries from the following links
  http://librarymanager/all#BMP180&Adafruit
  http://librarymanager/all#HTU21D&Adafruit
  http://librarymanager/all#TSL2561&Adafruit
  http://librarymanager/all#adafruit&sensor&abstraction

  Since the Sigfox network can send a maximum of 120 messages per day (depending on your plan)
  we'll optimize the readings and send data in compact binary format

  This example code is in the public domain.
*/

#include "ArduinoLowPower.h"
#include "SigFox.h"
#include "Adafruit_HTU21DF.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP085_U.h"
#include "Adafruit_TSL2561_U.h"
#include "conversions.h"

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

#define STATUS_OK     0
#define STATUS_BMP_KO 1
#define STATUS_HTU_KO 2
#define STATUS_TSL_KO 4

typedef struct sigfox_message {
  uint8_t status;
  int16_t moduleTemperature;
  int16_t bmpTemperature;
  uint16_t bmpPressure;
  uint16_t htuHumidity;
  uint16_t tlsLight;
  uint8_t lastMessageStatus;
} SigfoxMessage;

// stub for message which will be sent
SigfoxMessage msg;

void setup() {

  if (!SigFox.begin()) {
    //something is really wrong, try rebooting
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  // Configure the sensors and populate the status field
  if (!bmp.begin()) {
    msg.status |= STATUS_BMP_KO;
  }

  if (!htu.begin()) {
    msg.status |= STATUS_HTU_KO;
  }

  if (!tsl.begin()) {
    msg.status |= STATUS_TSL_KO;
  } else {
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  }
}

void loop() {
  // Every 15 minutes, read all the sensors and send them
  // Let's try to optimize the data format
  // Only use floats as intermediate representaion, don't send them directly

  float temperature = SigFox.getTemperatureInternal();
  msg.moduleTemperature = convertoFloadToInt16(temperature, 60, -60);

  sensors_event_t event;

  bmp.getEvent(&event);
  if (event.pressure) {
    msg.bmpPressure = convertoFloadToUInt16(event.pressure, 10000);
    bmp.getTemperature(&temperature);
    msg.bmpTemperature = convertoFloadToInt16(temperature, 60, -60);
  }

  tsl.getEvent(&event);
  if (event.light) {
      msg.tlsLight = convertoFloadToUInt16(event.light, 100000);
  }

  float humidity = htu.readHumidity();
  msg.htuHumidity = convertoFloadToUInt16(humidity, 110);

  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.getStatus();
  delay(1);

  msg.lastMessageStatus = (uint8_t) SigFox.send((uint8_t*)&msg, 12);

  SigFox.end();

  LowPower.sleep(60*60*1000);
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
