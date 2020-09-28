
#include <Arduino.h>
#include "A4988.h"

// change this to the number of steps on your motor
#define MOTOR_STEPS 200
#define RPM 10
#define MICROSTEPS 16
// configure the pins connected for motor
#define DIR 2
#define STEP 3
#define ENABLE 4
#define MS1 5
#define MS2 6
#define MS3 7
// configure sensor pins
int ledPin = 13; // choose the pin for the LED
int sensorIn = 8; // choose the input pin (for PIR sensor)
int pirState = LOW; // we start, assuming no motion detected
int val = 0; // variable for reading the pin status



A4988 stepper1(MOTOR_STEPS, DIR, STEP,MS1,MS2,MS3);


// the previous reading from the analog input
void setup() {
  Serial.begin(9600);
  
  //sensor set up
  pinMode(sensorIn, INPUT);
  pinMode(ledPin,OUTPUT);
  //motor set up
  pinMode(ENABLE,OUTPUT); 
  digitalWrite(ENABLE,LOW); // Set Enable low
  stepper1.begin(RPM,MICROSTEPS); // RPM, 1R travel 18 inch
}

void runMotor(){
  unsigned wait_time_micros = stepper1.nextAction();
  if (wait_time_micros <=0){
    stepper1.disable();
    digitalWrite(ENABLE,HIGH);
    digitalWrite(ENABLE,LOW);
    stepper1.startRotate(90);
  }
}
void loop() {

  val = digitalRead(sensorIn);
  if (val == HIGH && stepper1.nextAction() <= 0){
    digitalWrite(ledPin,HIGH);
    if (pirState == LOW) {
      Serial.println("Motion Detected");
      pirState = HIGH;
      runMotor();

    }
  }else if (stepper1.nextAction() <= 0){
      digitalWrite(ledPin,LOW);
      if (pirState == HIGH) {
        Serial.println("Motion Ended.");
        pirState = LOW;
        stepper1.disable();
        digitalWrite(ENABLE,HIGH);
      }
  }
}

/*
  unsigned wait_time_micros = stepper1.nextAction();
  if (wait_time_micros <=0){
    stepper1.disable();
    digitalWrite(ENABLE,HIGH);
    delay(2000);
    digitalWrite(ENABLE,LOW);
    stepper1.startRotate(90);
  }
  */
