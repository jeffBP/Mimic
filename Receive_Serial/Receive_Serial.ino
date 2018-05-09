

//Default baud speed for communication
#define BAUD 115200
#include <Servo.h>
//macro for on/off
Servo rightFore;
Servo rightElbow;
int shoulAngle;
int elbowAngle;

void setup(){
  Serial.begin(BAUD);
  rightFore.attach(6);
  rightElbow.attach(11);
  rightFore.write(100);
  rightElbow.write(0);
  
}

int stringToInt(String str) {
  int len = str.length();
  int sum = 0;
  for (int i = 0; i < len; i++) {
    char digit = str[i];
    int num = digit - '0';
    num = num * pow(10, len - i-1);
    sum += num;
  }
  return sum;
}

int getDelimit(String str) {
  String delimiter = ", ";
  int pos = str.indexOf(',');
  return pos;

}

void getAngs(String str){
  int pos = getDelimit(str);
  String ang1 = str.substring(0, pos);
  String ang2 = str.substring(pos + 2, str.length());
  shoulAngle = stringToInt(ang1);
  elbowAngle = stringToInt(ang2);

}



void loop(){
  char delim = ',';
  String input;
  char buff[2];
  int ang1;
  int ang2;
  //If any input is detected in arduino
  if(Serial.available() > 0){
    //read the whole string until '\n' delimiter is read 
      
      ang1 = Serial.parseInt();
      Serial.readBytes(buff, 2);
      ang2= Serial.parseInt();
      rightFore.write(ang1);
      rightElbow.write(ang2);
      delay(20);
      Serial.println("moved.");
   

  }
}
