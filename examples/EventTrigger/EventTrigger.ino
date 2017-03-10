/*
  SigFox Event Trigger tutorial

  This sketch demonstrates the usage of a MKRFox1200
  to build a battery-powered alarm sensor with email notifications

  A couple of sensors (normally open) should we wired between pins 1 and 2 and GND.

  This example code is in the public domain.
*/

#include "SigFox.h"
#include "ArduinoLowPower.h"

// Comment the following line to enable continuous mode
// and disable serial prints
#define DEBUG

volatile int alarm_source = 0;

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {}
#endif

  if (!SigFox.begin()) {
    //something is really wrong, try rebooting
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

#ifdef DEBUG
  // Enable debug prints and LED indication if we are testing
  SigFox.debug(true);
#endif

  // attach pin 1 and 2 to a switch and enable the interrupt on voltage falling event
  pinMode(1, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(1, alarmEvent1, FALLING);

  pinMode(2, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(2, alarmEvent2, FALLING);
}

void loop()
{
  // Sleep until an event is recognized
  LowPower.sleep();

  // if we get here it means that an event was received

  SigFox.begin();

#ifdef DEBUG
  Serial.println("Alarm event on sensor " + String(alarm_source));
#endif

  delay(100);

  // 3 bytes (ALM) + 4 bytes (ID) + 1 byte (source) < 12 bytes
  String to_be_sent = "ALM" + SigFox.getID() +  String(alarm_source);

  int ret = SigFox.send(to_be_sent);

  // shut down module, back to standby
  SigFox.end();

#ifdef DEBUG
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.getStatus(SIGFOX));
  Serial.println(SigFox.getStatus(ATMEL));

  // Loop forever if we are testing for a single event
  while (1) {};
#endif
}

void alarmEvent1() {
  alarm_source = 1;
}

void alarmEvent2() {
  alarm_source = 2;
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
