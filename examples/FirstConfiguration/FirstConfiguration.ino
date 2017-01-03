/*
  SigFox Getting started

  This sketch demonstrates the usage of MKR3000 SigFox module.
  Since the board is designed with lowe power in mind, it depends directly on ArduinoLowPower library

  This example code is in the public domain.
*/

#include "SigFox.h"
#include "ArduinoLowPower.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

  // Uncomment this line and comment begin() if you are working with a custom board
  //if (!SigFox.begin(SPI1, 30, 31, 33, 28, LED_BUILTIN)) {
  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  SigFox.debug(true);

  String version = SigFox.getSigVersion();
  String ID = SigFox.getID();
  String PAC = SigFox.getPAC();

  // Display module informations
  Serial.println("MKR3000 Sigfox first configuration");
  Serial.println("SigFox FW version " + version);
  Serial.println("ID  = " + ID);
  Serial.println("PAC = " + PAC);

  Serial.println("");

#if ARDUINO >= 10801
  Serial.println("Click <a href=\"https://backend.sigfox.com/new?pac=" + PAC + "?id=" + ID +  "\">here</a> to register the board on SigFox network");
#else
  Serial.println("Register your board on https://backend.sigfox.com with provided ID and PAC");
#endif

  // Set region and transmission mode
  // if TX - only mode is selected, function receive() will not be available
  // Possible regions are US and EU
  SigFox.setMode(EU, TXRX);

  // Send the module to the deepest sleep
  SigFox.end();

  // Example of message that can be sent
  //sendString("Hello world!");

  // Example of send and read response
  sendStringAndGetResponse("Hello world!");
}

void loop()
{
  // Uncomment to sleep for 10minutes
  // LowPower.sleep(10 * 60 * 1000);
  // Check if something happened and eventually report back
  // result = do_the_check();
  // if (result == true)
  //  sendString("something")
}

void sendString(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.getStatus();
  delay(1);

  int ret = SigFox.send(str);  // send buffer to SIGFOX network
  if (ret > 0)
    Serial.println("No transmission");
  else
    Serial.println("Transmission ok");

  Serial.println(SigFox.getStatus(SIGFOX));
  Serial.println(SigFox.getStatus(ATMEL));
  SigFox.end();
}

void sendStringAndGetResponse(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.getStatus();
  delay(1);

  int ret = SigFox.receive(str);  // send buffer to SIGFOX network
  if (ret > 0)
    Serial.println("No transmission");
  else
    Serial.println("Transmission ok");

  Serial.println(SigFox.getStatus(SIGFOX));
  Serial.println(SigFox.getStatus(ATMEL));

  Serial.println("Response from server:");
  while (SigFox.available()) {
    Serial.print("0x");
    Serial.println(SigFox.read(), HEX);
  }
  Serial.println();

  SigFox.end();
}
