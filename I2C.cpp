#include "I2C.h"
#include "Wire.h"
#include "Arduino.h"
#include "env.h"

bool isI2CBusStuck() {
  Wire.end();
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  delay(5);

  bool sdaLow = digitalRead(SDA_PIN) == LOW;
  bool sclHigh = digitalRead(SCL_PIN) == HIGH;

  // Turn the pins back to output
  Wire.begin(SDA_PIN, SCL_PIN);

  return sdaLow && sclHigh; // SDA low and SCL high indicates a stuck bus
}

int numI2CBusStuck = 0;
unsigned long lastI2CBusStuckAtMillis = 0;

bool recoverI2CBus() {
  Wire.end();
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, OUTPUT);
  delay(5);

  // Generate 9 clock pulses to release the SDA line
  for (int i = 0; i < 9; i++) {
    digitalWrite(SCL_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(SCL_PIN, LOW);
    delayMicroseconds(5);
  }

  // Generate a STOP condition
  pinMode(SDA_PIN, OUTPUT);
  digitalWrite(SDA_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(SCL_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(SDA_PIN, HIGH);

  // Reinitialize the I2C bus
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C bus recovery complete.");
  numI2CBusStuck++;
  lastI2CBusStuckAtMillis = millis();
  return true;
}

int getNumI2CBusStuck() {
  return numI2CBusStuck;
}

unsigned long getLastI2CBusStuckAtMillis() {
  return lastI2CBusStuckAtMillis;
}