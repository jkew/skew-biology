#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "IRremote.h"


#define OLED_RESET 4
#define ONE_WIRE_BUS 13

int relayPin = 12;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_SSD1306 display(OLED_RESET);

// IR
int receiver = 11;
IRrecv irrecv(receiver);
decode_results results;

// Temp
double set_temp = -1;
int last_delta = 0;
unsigned long pid_window_start;
int pid_window_size = 5000;
double curr_temp, pid_output;
double max_temp, min_temp, last_min, last_max;
double last_temp[5];
unsigned int last_i = 0;

//Define the aggressive and conservative Tuning Parameters
// 300 => 31c
// 350 => 
// 400 =>
// 500 => 32
// 1000 => 34
// 1500 => 35.38
// 3000 => 36c
// 4500 =36.44
double aggKp=8000, aggKi=.9, aggKd=0.1; 
double consKp=1, consKi=0.05, consKd=0.25;

PID myPID(&curr_temp, &pid_output, &set_temp,aggKp,aggKi,aggKp,P_ON_E, DIRECT);

void translateIR()
{

  switch(results.value)

  {
    case 0xFF22DD: if (set_temp > 37.0) set_temp=37.0; else set_temp = curr_temp; last_delta = 0;break;
    case 0xFFC23D: if (set_temp < 37.0) set_temp=37.0; else set_temp = 50.0;  last_delta = 0;break; // FF
    case 0xFFE01F: set_temp= set_temp - 0.1;  last_delta = -0.1;break; // DOWN
    case 0xFF906F: set_temp= set_temp + 0.1 ;  last_delta = 0.1; break; // UP
    case 0xFFFFFFFF: set_temp= set_temp + last_delta;break;  
  }
}


void setup() {
  pinMode(relayPin, OUTPUT);
  sensors.begin();
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Setup");
  pid_window_start = millis();

}


void status(char *msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Current: ");
  display.print(curr_temp);
  display.print("c");
  display.setCursor(0,9);
  display.print("Min/Max: ");
  display.print(min_temp);
  display.print("-");
  display.print(max_temp);
  display.setCursor(0,20);
  display.println(msg);
  display.setCursor(0,30);
  display.print("PID % ");
  display.print((int) ((pid_output / pid_window_size) * 100));
  display.setCursor(0,40);
  display.print("Set Temp: ");
  display.print(set_temp);
  display.print("c");
  display.setCursor(0,50);
  display.print((int)aggKp);
  display.print(':');
  display.print((int)aggKi);
  display.print(':');
  display.print((int)aggKd);
  display.display();
}

void pid_control(double gap, unsigned long now) {
 // if(gap < 0.1) {  //we're close to setpoint, use conservative tuning parameters
 //   myPID.SetTunings(consKp, consKi, consKd);
 // } else {
    myPID.SetTunings(aggKp, aggKi, aggKd);
 // }

  if(pid_output > (now - pid_window_start)) { 
    digitalWrite(relayPin,HIGH);
    status("HIGH - PID");
  } else {
    digitalWrite(relayPin,LOW);
    status("LOW  - PID");
  }
}

void loop() {
  sensors.requestTemperatures();
  last_temp[last_i] = curr_temp;
  last_i = (last_i + 1) % 5;
  
  curr_temp = sensors.getTempCByIndex(0);

  // init
  if (set_temp < 0) {
    set_temp = 37.0;
    max_temp = curr_temp;
    min_temp = curr_temp;
    for (int i = 0; i < 5; i++) {
       last_temp[i] = curr_temp;
    }
    myPID.SetOutputLimits(0, pid_window_size);
    //turn the PID on
    myPID.SetMode(AUTOMATIC);
  }

  double ave_last_temp = 0;
  for (int i = 0; i < 5; i++) {
     ave_last_temp += last_temp[i];
  }
  ave_last_temp = ave_last_temp / 5;
  
  // Climbing from minima
  if (ave_last_temp < curr_temp) {
    if (ave_last_temp > last_min) {
      min_temp = last_min;
    }

    // Climbing to maxima
    if (ave_last_temp > set_temp) {
      last_max = ave_last_temp;
    }
      
  }
  
  // Falling from maxima
  if (ave_last_temp > curr_temp) {
    if (ave_last_temp < last_max) {
      max_temp = last_max;
    }

    // Falling to minima
    if (ave_last_temp < set_temp) {
      last_min = ave_last_temp;
    }
  }
  
  double gap = abs(set_temp-curr_temp); //distance away from setpoint
  boolean ramping = max_temp < ( set_temp - 1 );

  unsigned long now = millis();
  //if (ramping) {
    // during ramping; reset the pid_window_start;
//    pid_window_start = now;
//  } else {
    // compute the pid once we are finished with the initial ramp
    myPID.Compute();
    if((now - pid_window_start) > pid_window_size) { //time to shift the Relay Window
      pid_window_start += pid_window_size;
    }
 // }
  
  // At < 1c begin PID control; severe guard rails
  //if (gap < 1 && !ramping) {
    pid_control(gap, now);
  //} else {
  //  if (curr_temp < set_temp) {
  //    digitalWrite(relayPin,HIGH);
  //    status("HIGH - RAMP");
  //  } else {
  //    digitalWrite(relayPin,LOW);
  //    status("LOW  - RAMP"); 
  //  }
  //}
  
  if (irrecv.decode(&results)) {
    translateIR(); 
    irrecv.resume(); // receive the next value
  } 
}
