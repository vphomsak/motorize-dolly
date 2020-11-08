
#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"
#include <nRF24L01.h>

//motor pin
const int right_pwm = 3;
const int left_pwm = 5;
const int dutyCycle = 200; //pwm 0-255 - how much volage in the motor- smaller value less power 

//Movements
unsigned long unwind_time = 5000; // 10 sec the pause before it turns (goes back)
unsigned long unwind_start = 0; // the time the delay started
bool unwinding = false; // true if still waiting for delay to finish
const int default_step = 2;
const int turn_step = 4; 
const int duration_per_step = 50; // millisecond 2000 the peroid of motor on
const int delay_between_step = 100; // time between each step - less the smoother the greater the more pause



// configure sensor pins
const int sensorCenter = 9; // choose the input pin (for PIR sensor)
const int sensorSide = 8;
int pirState = LOW; // we start, assuming no motion detected
int val = 0; // variable for reading the pin status
bool sensor_pause = false;

const int led_tx = 2;const int led_rx = 12;

//controller button
const int button_forward = 3;
const int button_backward = 4;
const int button_right_turn = 8;
const int button_left_turn = 9;

RF24 radio(6,7); //6-CS 7-CSN


typedef enum {controller, left_wheel, right_wheel} role_type;
typedef enum {forward, backward,right_turn,left_turn,pause_sensor,unpause_sensor} motion_type;

byte nodes[][6] = {"1Node","2Node","3Node"};

//**** Setting the board role before upload the code to ardui
role_type role = right_wheel; //left_wheel; //controller;

motion_type last_motion;

//Function to configure the radio
void configureRadio(){
 
  if (!radio.begin()){
    Serial.println("Configuration incorrect, start up failed!!!");
  }
 
  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(80);
  
  // Open a writing and reading pipe on each radio, with opposite addresses

  for(int i =0;i<3;i++)
    radio.openReadingPipe(i+1,nodes[i]);

  radio.openWritingPipe(nodes[0]);
  
  // Start the radio listening for data
  radio.startListening();

  radio.printDetails();
}



// the previous reading from the analog input
void setup() {
  Serial.begin(115200);

  //dc motor
  pinMode(right_pwm,OUTPUT);
  pinMode(left_pwm,OUTPUT);
  
  //sensor set up
  pinMode(sensorCenter, INPUT);
  pinMode(sensorSide, INPUT);

  if (role == controller){
    pinMode(button_right_turn, INPUT);
    pinMode(button_left_turn, INPUT);
  }

  digitalWrite(sensorCenter,LOW);
  digitalWrite(sensorSide, LOW);
  

  pinMode(led_tx,OUTPUT);
  digitalWrite(led_tx,LOW);
  pinMode(led_rx,OUTPUT);
  digitalWrite(led_rx,LOW);
 
  printf_begin();
  configureRadio();  


}

void runMotor(){
  Serial.println("Run Motor");
}

void broadcastMessage(motion_type motion){
  Serial.print("Broadcast message."); Serial.println(motion);
  radio.stopListening();
  radio.write(&motion,sizeof(motion_type));
  radio.startListening();
  displayMessage(motion);
}

void displayMessage(motion_type motion){
  Serial.print("Motion received"); Serial.println(motion);
  int pin = led_tx;
  switch(motion){
    case forward:
      blink(pin,2);
      break;
    case left_turn:
      blink(pin,3);
      break;
    case right_turn:
      blink(pin,4);
      break;
    case backward:
      blink(pin,5);
  } 
}

void digestMessage(motion_type motion){
  Serial.print("Digest Message:"); Serial.println(motion);
  switch(motion){
    case forward:
      move_forward(default_step);
      break;
    case left_turn:
      turn_left();
      break;
    case right_turn:
      turn_right();
      break;
    case backward:
      move_backward(default_step);
    case pause_sensor:
      sensor_pause = true;
    case unpause_sensor:
      sensor_pause = false;
    } 
}

/*
void moveDolly(motion_type motion){

  Serial.print("Motion received: "); Serial.println(motion);
  switch(motion){
    case forward:
      stepping(10,100,right_pwm,left_pwm);
      break;
    case left_turn:
      blink(3);
      break;
    case right_turn:
      blink(4);
      break;
    case backward:
      stepping(10,100,left_pwm,right_pwm);

  }
}
*/

void stop(){
  analogWrite(left_pwm,0);
  analogWrite(right_pwm,0);
//  digitalWrite(left_pwm,LOW);
//  digitalWrite(right_pwm,HIGH);
}

void move_forward(int step){
  stepping(step,duration_per_step,right_pwm,left_pwm);
}

void move_backward(int step){
  stepping(step,duration_per_step,left_pwm, right_pwm);
}

void turn_left() {
  if (role == left_wheel)
    move_backward(turn_step);
  else
    move_forward(turn_step);
}

void turn_right(){
  if (role == left_wheel)
    move_forward(turn_step);
  else
    move_backward(turn_step);
}
void stepping(int step, int duration, int disable_pin, int enable_pin){
  analogWrite(disable_pin, 0);
  delayMicroseconds(100);
  for (int i = 0; i < step; i++){
    analogWrite(enable_pin, dutyCycle); //running
    delay(duration);
    stop();
    delay(delay_between_step);
  }
 
  
}
void blink(int pin, int n){
  for (int i=0;i<n;i++){
    digitalWrite(pin,HIGH);
    delay(50);
    digitalWrite(pin,LOW);
    delay(400);
  }
}

bool checkSensor(int sensor){
  val = digitalRead(sensor);
    if (val == HIGH){
      digitalWrite(led_tx,HIGH);
      if (pirState == LOW) {
        Serial.print("Motion Detected at Pin:"); Serial.println(sensor);
        pirState = HIGH;
        if(sensor == sensorCenter){
          broadcastMessage(forward);
          move_forward(default_step);
          last_motion = forward;
        }else{
          motion_type mt = (role == left_wheel? left_turn : right_turn);
          broadcastMessage(mt); //role == left_wheel? left_turn : right_turn);
          if (mt == left_turn)
            turn_left();
          else
            turn_right();
          last_motion = mt;
        }
      }
   }else{
      digitalWrite(led_tx,LOW);
      if (pirState == HIGH) {
        Serial.println("Motion Ended.");
        pirState = LOW;      
      }
  }  
  return val;
}

void loop() {
  
  if (unwinding){
    if (millis() - unwind_start > unwind_time){
      Serial.print("Unwinding:"); Serial.println(last_motion);
      if (last_motion == forward){
        broadcastMessage(backward);
        move_backward(default_step);
      }else if (last_motion == left_turn){
        broadcastMessage(right_turn);
        turn_right();
      }else if (last_motion == right_turn){
        broadcastMessage(left_turn);
        turn_left();
      }
      unwinding = false;
      broadcastMessage(unpause_sensor);
      
    }
  }else if (role != controller && !sensor_pause){
    if (!checkSensor(sensorSide))
      checkSensor(sensorCenter);
    if (pirState == HIGH){
      unwinding = true;
      unwind_start = millis();
      broadcastMessage(pause_sensor);
    }
  }else { //controller role
    if (digitalRead(button_forward) == HIGH)
      broadcastMessage(forward);
    else if (digitalRead(button_backward) == HIGH)
      broadcastMessage(backward);
    else if (digitalRead(button_right_turn) == HIGH)
      broadcastMessage(right_turn);
    else if (digitalRead(button_left_turn) == HIGH)
      broadcastMessage(left_turn);
    delay(5000);
  }
  
  if (radio.available()){
    motion_type motion ;
    radio.read(&motion,sizeof(motion_type));
    displayMessage(motion);
    if(!unwinding)
      digestMessage(motion);
    
 }

 

  
}

/*
 * val = digitalRead(sensorIn);
    if (val == HIGH){
      digitalWrite(ledPin,HIGH);
      if (pirState == LOW) {
        Serial.println("Motion Detected");
        pirState = HIGH;
        broadcastMessage(forward);
        runMotor();
      }
   }else{
      digitalWrite(ledPin,LOW);
      if (pirState == HIGH) {
        Serial.println("Motion Ended.");
        pirState = LOW;      
      }
  }
 */

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
