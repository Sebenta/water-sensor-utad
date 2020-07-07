#include <MKRWAN.h>
#include "arduino_secrets.h"

String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

// Select your region (AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID)
_lora_band region = EU868;

LoRaModem modem(Serial1);

unsigned long time;
unsigned long timeUpload;

uint16_t voltage = 0;
uint16_t drops = 0;

float dropvalue = 0;
uint8_t txBuffer[2];

void setup()
{

  Serial.begin(115200);
  pinMode(A0, INPUT);
  time = millis();
  timeUpload = millis();

  while (!Serial)
  {
  }

  if (!modem.begin(region))
  {
    Serial.println("Failed to start module");
    while (1)
    {
    }
  };

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected)
  {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1)
    {
    }
  }

  modem.minPollInterval(60);
  // NOTA: modem só permite enviar msg no mínimo a cada 2 min
}

void loop()
{

  // Read the voltage value
  voltage = analogRead(A0);

  // If the voltage is greater than 1 volt, it counts a drop
  if (voltage > 210)
    drops++;

  // Average drops per minute (60000ms) - Maximum expected value: 1200 drops
  if (millis() - time > 60000)
  {
    dropvalue = (float)drops * 0.6 + (float)dropvalue * 0.4;
    drops = 0;
    time = millis();
  }

  // Send values to "the network of things" hourly (3_600_000ms)
  if (millis() - timeUpload > 3600000)
  {

    // Data processing
    int aux = (int)(dropvalue + 0.55);
    txBuffer[0] = (aux >> 8) & 0xff;
    txBuffer[1] = aux & 0xff;

    //Serial.println(txBuffer[0], HEX);
    //Serial.println(txBuffer[1], HEX);

    // Sending data
    int err;
    modem.beginPacket();
    modem.write(txBuffer);
    err = modem.endPacket(true);
    if (err > 0)
    {
      Serial.println("Message sent correctly!");
    }
    else
    {
      Serial.println("Error sending message :(");
      Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
      Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
    }

    // Reset timer
    timeUpload = millis();
  }
  delay(50);
}
