#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <Base64.h>
#include "LowPower.h"
#include <avr/wdt.h>

/*
  The encrypted message is in this format:
  
  +--------------+----------------+---------------------+
  | Addr-Counter | Encrypted Data | Authentication Code |
  +--------------+----------------+---------------------+
  
  Where:
  Addr-Counter = Address + part of initialization vector (1 byte address, 3 byte message counter)
  Encrypted data is encrypted using ChaCha20 (IETF variant)
  The authentication code is calculated from the encrypted data using Poly1305 (bottom 4 bytes used)
  
  The IV is 12 bytes and is incremented for every message sent (only top 1 and bottom 3 sent)
  When the IV exceeds 2^24 the key needs to be changed (or both ends need to agree that the unsent IV bytes are going to change)
*/

#define DEBUG 1

#define BUTTON_PIN 7
#define LDR_POWER 8
#define DS18B20_ONE_WIRE_BUS 9
#define HC12_TX 11
#define HC12_RX 12
#define LDR_PIN A0

#define SEND_ADDR 1

#define MAX_MSG_LEN 32
#define CIPHER_MSG_BYTES (4 + 4)
#define BASE64_MULTIPLIER(n) (((4 * (n) / 3) + 3) & ~3)

#define MAX_PLAINTEXT_SIZE  (MAX_MSG_LEN + 1)
#define MAX_CIPHERDATA_SIZE (MAX_MSG_LEN + CIPHER_MSG_BYTES)
#define MAX_BASE64TEXT_SIZE (BASE64_MULTIPLIER(MAX_CIPHERDATA_SIZE) + 1)
//#define MAX_BASE64TEXT_SIZE (MAX_CIPHERDATA_SIZE * 3 + 1)

OneWire ds18b20_OneWire(DS18B20_ONE_WIRE_BUS);
DallasTemperature ds18b20(&ds18b20_OneWire);

SoftwareSerial HC12(HC12_TX, HC12_RX);

short state = 0;
#ifdef DEBUG
short sleep4Count = 1;
#else
short sleep4Count = 1; //15;
#endif

void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_POWER, OUTPUT);
  HC12Setup();
  ds18b20.setWaitForConversion(false);
  ds18b20.begin();
  DebugSerialSetup();
}

void loop()
{
  char plainText[MAX_PLAINTEXT_SIZE];
  char cipherData[MAX_CIPHERDATA_SIZE];
  char base64Text[MAX_BASE64TEXT_SIZE];

  DebugSerialPrint("State = ");
  DebugSerialPrintln(state);

  if (state == 1)
  {
    ds18b20.requestTemperatures();
    state++;
    PowerDown(SLEEP_500MS);
  }
  else if (state == 2)
  {
    HC12PowerUp();
    state++;
    PowerDown(SLEEP_250MS);
  }
  else if (state == 3)
  {
    GetTempLightData(plainText);
    int cipherLen = EncryptPlainText(cipherData, plainText, strlen(plainText));
    Base64.encode(base64Text, cipherData, cipherLen);
    //BytesToString(base64Text, cipherData, cipherLen);
    DebugSerialPrintln(plainText);
    IncrementIV();
    HC12.println(base64Text);
    HC12.flush();
    PowerDown(SLEEP_60MS);
    HC12PowerDown();
    PowerDown(SLEEP_15MS);
    state = 1;
    for (int i = 0; i < sleep4Count; i++)
    {
      PowerDown(SLEEP_4S);
    }
  }
  else
  {
    DebugSerialPrintln("Off");
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(10);
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        DebugSerialPrintln("On");
        state = 1;
      }
    }
    PowerDown(SLEEP_1S);
  }
}

void GetTempLightData(char* str)
{
  digitalWrite(LDR_POWER, HIGH);
  int ldr = analogRead(LDR_PIN);
  digitalWrite(LDR_POWER, LOW);
  float celcius = ds18b20.getTempCByIndex(0);
  long vcc = readVcc();
  sprintf(str, "T%d.%01d;L%d;V%d;\0", (int)celcius, (int)(celcius*10)%10, ldr, vcc);
}
