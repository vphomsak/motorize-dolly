
#include <Arduino.h>

// configure sensor pins
int ledPin = 13; // choose the pin for the LED
int sensorIn = 8; // choose the input pin (for PIR sensor)
int pirState = LOW; // we start, assuming no motion detected
int val = 0; // variable for reading the pin status




// the previous reading from the analog input
void setup() {
  Serial.begin(9600);
  
  //sensor set up
  pinMode(sensorIn, INPUT);
  pinMode(ledPin,OUTPUT);
  //motor set up
  pinMode(ENABLE,OUTPUT); 
  digitalWrite(ENABLE,HIGH); // disable powering of controller
}

void runMotor(){
  Serial.println("Run Motor");
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
