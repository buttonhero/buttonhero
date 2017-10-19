
#include <SPI.h>

//LiquidCrystal lcd(8);
const unsigned short int latch = 12;
const unsigned short int data = 11;
const unsigned short int clck = 10;

const unsigned short int green = 1;
const unsigned short int yellow = 2;
const unsigned short int blue = 4;
const unsigned short int red = 8;
const unsigned short int white = 16;
const unsigned short int verySlow = 500;
const unsigned short int slow = 400;
const unsigned short int medium = 300;
const unsigned short int fast = 200;
const unsigned short int veryFast = 100;
const unsigned short int blazing = 50;
unsigned short int arraySize = 8;
unsigned short int i = 0;
//
const unsigned short int songArray[8][2] = {{green,veryFast},{yellow,veryFast},{red,fast},{yellow,fast},{red,veryFast},{blue,veryFast},{white,veryFast},{green,veryFast}};

void setup() {
 
  pinMode(latch,OUTPUT);
  pinMode(clck, OUTPUT);
  pinMode(data, OUTPUT);  
  digitalWrite(data, 0);
  digitalWrite(clck, 0);
  
}
  

void loop() {
  if(i < arraySize){

    colorRow(songArray[i][0],songArray[i][1]);
    i++;
  }
   
}

void colorRow(unsigned short int colorSeq, unsigned short int delayValue){
  for(int i = 0; i < 5; i++){
    digitalWrite(latch,LOW);
    shiftOut(data, clck, MSBFIRST, colorSeq);
    digitalWrite(latch,HIGH);
    delay(delayValue);
    
  }
}
