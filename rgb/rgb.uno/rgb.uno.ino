#include <Wire.h>

#include <Adafruit_TCS34725.h>


/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
//Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup(void) {
  Serial.begin(9600);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);

  if (tcs.begin()) {
    Serial.println("Temp,Lux,Red,Green,Blue,Clear");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  digitalWrite(7, LOW);
  // Now we're ready to get readings!
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;
  delay(9000);
  digitalWrite(6, HIGH);
  delay(1000);
  tcs.getRawData(&r, &g, &b, &c);
  digitalWrite(6, LOW);

  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  
  Serial.print(colorTemp, DEC); Serial.print(",");
  Serial.print(lux, DEC); Serial.print(",");
  Serial.print(r, DEC); Serial.print(",");
  Serial.print(g, DEC); Serial.print(",");
  Serial.print(b, DEC); Serial.print(",");
  Serial.print(c, DEC); Serial.print(",");
  Serial.println("");
}

