#define HC12_POWER 2
#define HC12_SET 10

void BytesToString(char* output, byte* bytes, int len)
{
  for (int i = 0; i < len; i++)
  {
    sprintf(output + i * 3, "%02x ", bytes[i]);
  }
}

// See: https://forum.arduino.cc/index.php?topic=356752.0

long readVcc()
{
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 112640L / result; // Back-calculate AVcc in 10s of mV, e.g. 5.2V = 520
  return result;
}

void HC12Setup()
{
  pinMode(HC12_SET, OUTPUT);
  pinMode(HC12_POWER, OUTPUT);
  HC12.begin(9600);
  HC12PowerDown();
}

void HC12PowerDown()
{
  digitalWrite(HC12_POWER, HIGH);
  digitalWrite(HC12_SET, LOW);
  digitalWrite(HC12_RX, LOW);
  digitalWrite(HC12_TX, LOW);
}

void HC12PowerUp()
{
  digitalWrite(HC12_SET, HIGH);
  digitalWrite(HC12_RX, HIGH);
  digitalWrite(HC12_TX, HIGH);
  digitalWrite(HC12_POWER, LOW);
}

void PowerDown(period_t sleepTime)
{
#ifdef DEBUG
  Serial.flush();
#endif
  LowPower.powerDown(sleepTime, ADC_OFF, BOD_OFF);
}

void DebugSerialSetup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void DebugSerialPrint(char* str)
{
#ifdef DEBUG
    Serial.print(str);
#endif
}

void DebugSerialPrintln(char* str)
{
#ifdef DEBUG
  Serial.println(str);
#endif
}

void DebugSerialPrintln(short n)
{
#ifdef DEBUG
  Serial.println(n);
#endif
}
