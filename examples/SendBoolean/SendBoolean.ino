#include <SigFox.h>
#define VALUE_TO_SEND 1
#define DEBUG 1
void setup() {

  if (DEBUG){
    Serial.begin(9600);
    while (!Serial) {};
  }

  if (!SigFox.begin()) {
    if (DEBUG){
      Serial.println("Sigfox module unavailable !");
    }
    return;
  }
  if (DEBUG){
    SigFox.debug();
    Serial.println("ID  = " + SigFox.ID());
  }
  delay(100);
  int ret = SigFox.sendBit(VALUE_TO_SEND);
  if (DEBUG){
    Serial.print("Status : ");
    Serial.println(ret);
  }
}

void loop(){}
