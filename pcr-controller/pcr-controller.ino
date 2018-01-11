#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

#define ONE_WIRE_BUS 13
#define FAN_PIN 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  // put your setup code here, to run once:
  pinMode(FAN_PIN, OUTPUT);
  sensors.begin();
  Serial.begin(9600);
  Serial.println("Setup");
}

void loop() {
  digitalWrite(FAN_PIN, HIGH);
  Serial.println("ON");
  // put your main code here, to run repeatedly:
  sensors.requestTemperatures();
  double temp1 = sensors.getTempCByIndex(0);
  double temp2 = sensors.getTempCByIndex(1);
  double temp3 = sensors.getTempCByIndex(2);
  Serial.print("Temps");
  Serial.print(temp1);
  Serial.print(" : ");
  Serial.print(temp2);
  Serial.print(" : ");
  Serial.print(temp3);
  Serial.println("");
  delay(5000);
  digitalWrite(FAN_PIN, LOW);
  Serial.println("OFF");
  delay(5000);
}
