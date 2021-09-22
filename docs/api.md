# Sigfox Library

## Sigfox Class

### `SigFox.begin()`

#### Description

Initializes the Sigfox library and module

#### Syntax

```
SigFox.begin();
```

#### Parameters
None


#### Returns
true if correctly configured, false otherwise

#### Example

```
#include <SigFox.h>
#include <ArduinoLowPower.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
}

void loop(){
}
```

### `SigFox.beginPacket()`

#### Description

Begins the process of sending a packet

#### Syntax

```
SigFox.beginPacket();
```

#### Parameters
None

#### Example

```
#include <SigFox.h>
#include <ArduinoLowPower.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
}

void loop(){
  SigFox.begin();
  SigFox.beginPacket();
  SigFox.print("123456789012");
  SigFox.endPacket();
  while(1);
}
```

### `SigFox.write()`

#### Description

Sends binary data to the SigFox's backend. This data is sent as a byte or series of bytes; to send the characters representing the digits of a number use the print() function instead.

#### Syntax

```
SigFox.write(val)
SigFox.write(str)
SigFox.write(buf, len)
```

#### Parameters
val: a value to send as a single byte

str: a string to send as a series of bytes

buf: an array to send as a series of bytes

len: the length of the buffer

### `SigFox.print()`

#### Description

Sends characters data to the SigFox's backend. This data is sent as a character or series of characters; to send the binary data use the write() function instead.

#### Syntax

```
SigFox.print(val)
SigFox.print(str)
SigFox.print(buf, len)
```

#### Parameters
val: a value to send characters representing the value

str: a string to send as a series of characters

buf: an array to send as a series of characters

len: the length of the buffer

### `SigFox.endPacket()`

#### Description

Called after writing SigFox data to the remote connection, completes the process of sending a packet started by beginPacket.

#### Syntax

```
SigFox.endPacket();
```

#### Return
Returns an int: 1 if the packet was sent successfully, 0 if there was an error


#### Example

```
#include <SigFox.h>
#include <ArduinoLowPower.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {};

  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }

  void loop() {
    SigFox.begin();
    SigFox.beginPacket();
    SigFox.print("123456789012");
    int ret = SigFox.endPacket();
    if (ret == 0)
      Serial.println("OK");
    else
      Serial.println("KO");
    while (1);
  }
```

### `SigFox.parsePacket()`

#### Description

Checks for the presence of a SigFox packet, and reports the size. parsePacket() must be called before reading the buffer with SigFox.read().

#### Syntax

```
SigFox.parsePacket()
```

#### Parameters
None


#### Returns
int: the size of a received SigFox packet

### `SigFox.statusCode()`

#### Description

Returns the protocol status code

#### Syntax

```
SigFox.statusCode(protocol);
```

#### Parameters

protocol: can be one of either:

- SSM
- ATMEL
- SIGFOX

#### Returns
the status code of the chosen protocol:

**SSM**

TBD

**Atmel**

Bit0: PA on/off indication
Bit6: System ready to operate (system ready event)
Bit5: Frame sent (frame ready event)
Bit4 to Bit1: Error code

- 0000: no error
- 0001: command error / not supported
- 0010: generic error
- 0011: frequency error
- 0100: usage error
- 0101: opening error
- 0110: closing error
- 0111: send error

**SIGFOX**

`0x00`: No error
`0x01`: Manufacturer error
`0x02`: ID or key error
`0x03`: State machine error
`0x04`: Frame size error
`0x05`: Manufacturer send error
`0x06`: Get voltage/temperature error
`0x07`: Close issues encountered
`0x08`: API error indication
`0x09`: Error getting PN9
`0x0A`: Error getting frequency
`0x0B`: Error building frame
`0x0C`: Error in delay routine
`0x0D`: callback causes error
`0x0E`: timing error
`0x0F`: frequency error

### `SigFox.AtmVersion()`

#### Description

Returns the Atm version

#### Syntax

```
SigFox.AtmVersion();
```

#### Returns
a String of 2 bytes containing the Atm version

### `SigFox.SigVersion()`

#### Description

Returns the module's firmware version

#### Syntax

```
SigFox.SigVersion();
```

#### Returns
a String of 2 bytes containing the SigFox version

### `SigFox.ID()`

#### Description

Returns the module ID. When a module is manufactured, a unique SigFox ID is recorded in its permanent memory. It is very important to keep and store the ID tray carefully, as it will be useful to insure the tracability of these devices and to register them on a SigFox Network Operator (SNO).

#### Syntax

```
SigFox.ID();
```

#### Returns
A String that contains the 4 bytes ID.

### `SigFox.PAC()`

#### Description

Returns the module PAC. For each module, a PAC key is a secret key corresponding to the Sigfox ID. The PAC key will be useful to register a device on a SigFox Network Operator (SNO). As opposed to the SigFox ID, a PAC key is not transferable and must be re-generated if the module's ownership is changed.

#### Syntax

```
SigFox.PAC();
```

#### Returns
A String that contains the 16 bytes PAC.

### `SigFox.reset()`

#### Description

Resets the module

#### Syntax

```
SigFox.reset();
```

#### Returns
None

### `SigFox.internalTemperature()`

#### Description

Returns the internal temperature sensor reading

#### Syntax

```
SigFox.internalTemperature();
```

#### Returns
a float representing the reading

### `SigFox.debug()`

#### Description

Enable debugging. Enabling the debugging all the power saving features are disabled and the led indicated as signaling pin (LED_BUILTIN as default) is used during transmission and receive events.

#### Syntax

```
SigFox.debug();
```

#### Parameters
None

### `SigFox.noDebug()`

#### Description

Disable debugging. Disabling the debugging all the power saving features are enabled and the led indicated as signaling pin (LED_BUILTIN as default) is not used during transmission and receive events.

#### Syntax

```
SigFox.debug();
```

#### Parameters
None

### `SigFox.end()`

#### Description

De-initializes the Sigfox library and module

#### Syntax

```
SigFox.end();
```

#### Returns
None

#### Example

```
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
  else
    SigFox.end();

}

void loop(){
}
```

### `peek()`

#### Description

Returns the next byte (character) of incoming serial data without removing it from the internal buffer. That is, successive calls to peek() will return the same character, as will the next call to read(). peek() inherits from the Stream utility class.

#### Syntax

```
SigFox.peek()
```

#### Parameters
None


#### Returns
the first byte of incoming serial data available (or -1 if no data is available) - int

### `available()`

#### Description

Get the number of bytes (characters) available for reading. This is data that's already arrived and stored in a receive buffer (which holds 8 bytes). available() inherits from the Stream utility class.

#### Syntax

```
SigFox.available()
```

#### Parameters
none


#### Returns
the number of bytes available to read

### `read()`

#### Description

Reads incoming SigFox data. read() inherits from the Stream utility class.

#### Syntax

```
SigFox.read()
```

#### Parameters
None



#### Returns
the first byte of incoming SigFox data available (or -1 if no data is available) - int

#### Example

```
/*
  SigFox First Configuration

  This sketch demonstrates the usage of MKRFox1200 SigFox module.
  Since the board is designed with low power in mind, it depends directly on ArduinoLowPower library

  This example code is in the public domain.
*/

#include <SigFox.h>
#include <ArduinoLowPower.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) {};

  // Uncomment this line and comment begin() if you are working with a custom board
  //if (!SigFox.begin(SPI1, 30, 31, 33, 28, LED_BUILTIN)) {
  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  SigFox.debug();

  String version = SigFox.SigVersion();
  String ID = SigFox.ID();
  String PAC = SigFox.PAC();

  // Display module information
  Serial.println("MKRFox1200 Sigfox first configuration");
  Serial.println("SigFox FW version " + version);
  Serial.println("ID  = " + ID);
  Serial.println("PAC = " + PAC);

  Serial.println("");

  Serial.print("Module temperature: ");
  Serial.println(SigFox.internalTemperature());

  Serial.println("Register your board on https://backend.sigfox.com/activate with provided ID and PAC");

  delay(100);

  // Send the module to the deepest sleep
  SigFox.end();

  Serial.println("Type the message to be sent");
  while (!Serial.available());

  String message;
  while (Serial.available()) {
    message += (char)Serial.read();
  }

  // Every SigFox packet cannot exceed 12 bytes
  // If the string is longer, only the first 12 bytes will be sent

  if (message.length() > 12) {
    Serial.println("Message too long, only first 12 bytes will be sent");
  }

  Serial.println("Sending " + message);

  // Remove EOL
  message.trim();

  // Example of message that can be sent
  // sendString(message);

  Serial.println("Getting the response will take up to 50 seconds");
  Serial.println("The LED will blink while the operation is ongoing");

  // Example of send and read response
  sendStringAndGetResponse(message);
}

void loop()
{
}

void sendString(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.print(str);

  int ret = SigFox.endPacket();  // send buffer to SIGFOX network
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));
  SigFox.end();
}

void sendStringAndGetResponse(String str) {
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.print(str);

  int ret = SigFox.endPacket(true);  // send buffer to SIGFOX network and wait for a response
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));

  if (SigFox.parsePacket()) {
    Serial.println("Response from server:");
    while (SigFox.available()) {
      Serial.print("0x");
      Serial.println(SigFox.read(), HEX);
    }
  } else {
    Serial.println("Could not get any response from the server");
    Serial.println("Check the SigFox coverage in your area");
    Serial.println("If you are indoor, check the 20dB coverage or move near a window");
  }
  Serial.println();

  SigFox.end();
}
```