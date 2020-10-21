
int dir = 1;

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
//  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  // Set initial rotation direction
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
}

void rampUp(){

  for(int i=0; i< 20;i++){
    if (i < 5){
      analogWrite(5,255);
      delay(25);
    }
    else{
      analogWrite(5,75);
      delay(20);
    }
      
  }
}
void loop() {
  // put your main code here, to run repeatedly:

  
  dir = -dir;
  if (dir < 0){
    
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    rampUp();
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    delay(1500);

    
  }else {
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    rampUp();
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    delay(1500);
  }
 

  /*
  rampUp();
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  delay(1500);
  */

}
