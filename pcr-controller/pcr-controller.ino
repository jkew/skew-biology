#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

#define ONE_WIRE_BUS 13
#define FAN_PIN 5
#define PWM_DIR_PIN 4
#define PWM_PIN 3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int heating = 1;
double lastAveTemp = 0.0;
void setup() {
  // put your setup code here, to run once:
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PWM_PIN,OUTPUT);
  pinMode(PWM_DIR_PIN,OUTPUT);
  sensors.begin();
  Serial.begin(9600);
  Serial.println("Setup");

  Serial.println("Starting TEC");
  analogWrite(PWM_PIN,255);
  digitalWrite(PWM_DIR_PIN, HIGH);
}

double get_temp() {
  sensors.requestTemperatures();
  double temp1 = sensors.getTempCByIndex(0);
  double temp2 = sensors.getTempCByIndex(1);
  double temp3 = sensors.getTempCByIndex(2);
  double aveTemp = (temp1 + temp2)/2.0;

  Serial.print("Ave Temp: ");
  Serial.print(aveTemp);
  Serial.print(" (");
  Serial.print(temp1);
  Serial.print(",");
  Serial.print(temp2);
  Serial.print(") Heat Sink: ");
  Serial.print(temp3);
  Serial.print(" c/sec: ");
  Serial.print((aveTemp - lastAveTemp));
  lastAveTemp = aveTemp;
  return aveTemp;
}

void goto_temp(double target_temp, int sec) {
  int sec_left = sec;
  double aveTemp = get_temp();
  int climbing = 0;
  int hit = 0;
  if (aveTemp < target_temp) {
    climbing = 1;
  }
  while (sec_left > 0) {
    aveTemp = get_temp();

    double delta = abs(aveTemp - target_temp);
    Serial.print(" PWM ");
    // Proportional Control < 5c Delta

    if (delta < 1.0) {
      hit = 1;
    }
    
    if (delta < 4.0) {
      Serial.print(((int) 50 * delta + 50));
      analogWrite(PWM_PIN,(int) 50 * delta + 50);
    } else {
      Serial.print(255);
      analogWrite(PWM_PIN,255);
    }

    if (delta > 5.0) {
      if (climbing) {
        digitalWrite(FAN_PIN, LOW);
      } else {
        digitalWrite(FAN_PIN, HIGH);
      }
    }
    
    if (aveTemp > (target_temp + 0.5)) {
        Serial.print(" COOL ");
        digitalWrite(PWM_DIR_PIN, LOW);

    } 

    if (aveTemp < (target_temp - 0.5)) {
      Serial.print(" HEAT ");
      digitalWrite(PWM_DIR_PIN, HIGH);
    }
    
    delay(1000);
    if (hit) {
      sec_left = sec_left - 1;
    }
    Serial.print(" sec: ");
    Serial.print(sec_left);
    Serial.print(" Target: ");
    Serial.println(target_temp);
  }
}

void loop() {
  goto_temp(95.0, 30);
  for (int i = 0; i < 25; i++) {
    goto_temp(95.0, 5);
    goto_temp(45.0, 10);
    goto_temp(72.0, 15);
  }
  goto_temp(4.0, 30);
}
