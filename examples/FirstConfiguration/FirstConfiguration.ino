#include "SigFox.h"    // Atmel SIGFOX shield library 
#include "ArduinoLowPower.h"

//#define Serial Serial1

#define lpb 60
char pbuff[lpb];

unsigned long timer;          // timer for sleeping
unsigned int count = 0;       // samples counter

boolean ishere;

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }
  delay(100);
  unsigned char *buffer;
  buffer = SigFox.getSigVersion();            // get version
  unsigned char ver[2] = {buffer[0], buffer[1]};
  buffer = SigFox.getID();                    // get ID
  unsigned char ID[4] = {buffer[0], buffer[1], buffer[2], buffer[3]};
  unsigned char *PAC = SigFox.getPAC();       // get PAC
  // display informations
  Serial.println("MKR3000 Sigfox first configuration");
  snprintf(pbuff, lpb, "SigFox FW version -V%d.%d\r\n", ver[0], ver[1]);
  Serial.println(pbuff);
  snprintf(pbuff, lpb, "ID  = %02X%02X%02X%02X\r\n", ID[3], ID[2], ID[1], ID[0]);
  Serial.println(pbuff);
  Serial.print("PAC = ");
  int i; for (i = 0; i < 8; i++) {
    snprintf(pbuff, lpb, "%02X", PAC[i]);
    Serial.print(pbuff);
  }
  Serial.println("");
  Serial.println("");
  Serial.println("Click here to register the board on SigFox network");

  SigFox.setEUMode();

  SigFox.end();               // switch off SIGFOX module
  
  sendString("Tanti auguri");
}

void loop()
{
}

void sendString(String str) {
  SigFox.begin();
  SigFox.getStatus();
  int ret = SigFox.sendMessage((unsigned char*)str.c_str(), str.length());  // send buffer to SIGFOX network
  Serial.println("message sent");
  Serial.println(SigFox.getStatusSig());
  Serial.println(SigFox.getStatusAtm());
  if (ret > 0) Serial.println("No transmission!");
  SigFox.end();
}
