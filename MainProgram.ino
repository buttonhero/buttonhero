#include <SPI.h> 
/* Wiring Guide */
//Arduino UNO
//Serial MP3 Player A     |  PINS//
//              RX              |   9
//              TX              |   8
//              VCC            |   5V
//              GND            |   GND
//JoyStick
//
//              SW         |  A0
//              X         |   1
//              Y         |   2
//              VCC       |   5V
//              GND        |  GND
//  LCD Array
//              
//              DATA       |  2
//              CLOCK      |  13
//              VCC        |  5V
//              GND        |  GND
//Buttons
//  
//              Yellow      | 3
//              Blue        | 4
//              Red         | A3
//              White       | A4
//              Green       | A5
//        ALL
//              VCC         | 5V
//              GND         | GND
 
#include <SoftwareSerial.h>
#include <TM1637Display.h>

//4 Digit Counter
#define CLK 13 //Set the CLK pin connection to the display
#define DIO 2//Set the DIO pin connection to the display

//All the pin definitions 
#define ARDUINO_RX 9 //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 8 //connect to RX of the module

//joystick pins
#define SW_PIN A0 // digital pin connected to switch output
#define X_PIN A1 // analog pin connected to X output
#define Y_PIN A2 // analog pin connected to Y output

//LED system pins
#define LATCH 12 //Latch Pin
#define DATA 11  //Data Pin
#define CLCK 10  //Clock Pin

//The Button pins
#define YB 3 //yellow button
#define BB 4 //blue button
#define RB A3 //red button
#define WB A4 //white button
#define GB A5 //green button 

SoftwareSerial myMP3(ARDUINO_RX, ARDUINO_TX);
TM1637Display display(CLK, DIO);  //set up the 4-Digit Display.


static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*basic commands sent to the serial mp3
Serial controlller takes hex code. These defines
put those hex codes as readable commands          */
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F //stop interruptting with a song, just stop the interlude

/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
#define DEV_TF 0X01
#define CMD_IC_MODE 0X35
#define CMD_SLEEP   0X03
#define CMD_WAKE_UP 0X02
#define CMD_RESET   0X05

/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43

/*Special commands*/
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31

#define CMD_SET_PLAY_MODE 0X33
#define ALL_CYCLE 0X00
#define SINGLE_CYCLE 0X01

#define CMD_PLAY_COMBINE 0X45 //can play combination up to 15 songs
/////////////////End Command Bytes///////////////////////

void sendCommand(int8_t command, int16_t dat );


#define GREEN 1
#define YELLOW 2
#define BLUE 4
#define RED 8
#define WHITE 16
#define VERYSLOW 500
#define SLOW 400
#define MEDIUM 300
#define FAST 200
#define VERYFAST 100
#define BLAZING 50
#define ARRAYSIZE 8 // Number of LED sequences in the song
#define SONGS 5 //number of songs
//
const unsigned short int songArray[ARRAYSIZE][2] = {{GREEN,VERYFAST},
                                                    {YELLOW,VERYFAST},
                                                    {RED,FAST},
                                                    {YELLOW,FAST},
                                                    {RED,VERYFAST},
                                                    {BLUE,VERYFAST},
                                                    {WHITE,VERYFAST},
                                                    {GREEN,VERYFAST}};

int score = 0;// the score. duh.
void setup() 
{
  display.setBrightness(0x0a);  //set the 4digit counter to maximum brightness
  
  //leds
  pinMode(LATCH,OUTPUT);
  pinMode(CLCK, OUTPUT);
  pinMode(DATA, OUTPUT);  
  digitalWrite(DATA, 0);
  digitalWrite(CLCK, 0);

  //buttons 
  pinMode(GB, INPUT_PULLUP);  
  pinMode(YB, INPUT_PULLUP);
  pinMode(BB, INPUT_PULLUP);  
  pinMode(RB, INPUT_PULLUP);
  pinMode(WB, INPUT_PULLUP);
  digitalWrite(GB, 0);
  digitalWrite(YB, 0);
  digitalWrite(BB, 0);  
  digitalWrite(RB, 0);
  digitalWrite(WB, 0);
  
  //joystick
  pinMode(SW_PIN, INPUT_PULLUP);
  digitalWrite(SW_PIN, HIGH);
  Serial.begin(9600);
  
  //serial baude rating for mp3 controller
  myMP3.begin(9600);
  delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card
  intro();
}
unsigned char gameState = 1; //1 = intro 2 = game 3 = finish 0 = select
unsigned char currSong = 1; //the song selected during the selection process

void loop() 
{
  Serial.print("Blue Button:  ");
  Serial.print(digitalRead(BB));
  Serial.print("\n");
    Serial.print("White Button:  ");
  Serial.print(digitalRead(WB));
  Serial.print("\n");
    Serial.print("Red Button:  ");
  Serial.print(digitalRead(RB));
  Serial.print("\n");
    Serial.print("Green Button:  ");
  Serial.print(digitalRead(GB));
  Serial.print("\n");
    Serial.print("Yellow Button:  ");
  Serial.print(digitalRead(YB));
  Serial.print("\n");
  //this is nessary for analog devices to have enough time to read.
  //Arduino specs says it takes at least 10 milliseconds in between reads.
  //Games state 2 already says a delay built in and this delay could interfer
  //with game play. 100 milliseconds was used to makes sure there was no error.
  if(gameState != 2){
  delay(100);
  }
  switch(gameState){
    case 1: //intro
      digitalWrite(LATCH,LOW); // sets them low to help with residual bugs
      shiftOut(DATA, CLCK, MSBFIRST, B11111111);  //Turn all the lights on. This helps with troubleshooting lights
      digitalWrite(LATCH,HIGH); 
      
      if(digitalRead(SW_PIN) == LOW){
        sendCommand(CMD_PLAY_FILE_NAME,0X022C); //says select a song
        delay(1500);
        sendCommand(CMD_PLAY_FILE_NAME,0X0202 + currSong); //plays song #. This should be 1 in everytime in intro 
        delay(500);
        sendCommand(CMD_PLAY_FILE_NAME,0X0101); // plays the first song
        gameState = 0;
      
        
      }
    break;  
    case 0: //select a song
      // so this delay makes it so a button select less then 1 second changes song while longer selects it
      if (digitalRead(SW_PIN) == LOW){
        delay(1000);
        if(digitalRead(SW_PIN) == LOW){
            sendCommand(CMD_PLAY_FILE_NAME,0X022B); // says "song selected"
            gameState = 2; // changes gameState to Game
            delay(2000);
            sendCommand(CMD_PLAY_FILE_NAME,0X0207); // says "5"
            delay(500);
            sendCommand(CMD_PLAY_FILE_NAME,0X0206); // says "4"
            delay(500);
            sendCommand(CMD_PLAY_FILE_NAME,0X0205); // says "3"
            delay(500); 
            sendCommand(CMD_PLAY_FILE_NAME,0X0204); // says "2"
            delay(600); 
            sendCommand(CMD_PLAY_FILE_NAME,0X0203); // says "1"
            delay(500);
            sendCommand(CMD_PLAY_FILE_NAME,0X0100 + currSong); // plays selected song       
        }
      else if(analogRead(X_PIN) > 900 || digitalRead(SW_PIN) == HIGH){
        
        if(currSong < SONGS) //checks if it isn't the last song
        {
            currSong++;
            sendCommand(CMD_PLAY_FILE_NAME,0X0202 + currSong); //says next song number
            delay(500);
            sendCommand(CMD_PLAY_FILE_NAME,0X0100 + currSong); //plays next song
            
        }  
        else // it is the last song so goes back to the beginning
        {
          currSong = 1;
          sendCommand(CMD_PLAY_FILE_NAME,0X0202 + currSong); //plays "1"
          delay(500);
          sendCommand(CMD_PLAY_FILE_NAME,0X0100 + currSong); // plays first song
            
        }
        
  
      }
      }
     //Uses joystcik X axis to select a song
     if(analogRead(X_PIN) < 100 ){
        if(currSong > 1)
        {
        currSong--;
        sendCommand(CMD_PLAY_FILE_NAME,0X0202 + currSong);
        delay(500);
        sendCommand(CMD_PLAY_FILE_NAME,0X0100 + currSong);
        
        }  
        else
        {
        currSong = SONGS;
        sendCommand(CMD_PLAY_FILE_NAME,0X0202 + currSong);
        delay(500);
        sendCommand(CMD_PLAY_FILE_NAME,0X0100 + currSong);
        
        }
       
      }
       
         
      break;
      case 2: //game
        for(unsigned char i = 0; i < ARRAYSIZE; i++){
          colorRow(songArray[i][0],songArray[i][1]);
          //this if statement allows you to pause and unpause the game
          if(digitalRead(SW_PIN) == LOW){
              mp3Basic(CMD_PAUSE);
              delay(1000);
              while(digitalRead(SW_PIN) == HIGH){
              }
              mp3Basic(CMD_PLAY);
              delay(1000); 
          }      
      }
      break;     
    //default:
        //nothing    
  }
    //use the joysticks y axis to control the volume
    if(analogRead(Y_PIN) > 900){
        mp3Basic(CMD_VOLUME_UP);
       delay(100);
    }
    if(analogRead(Y_PIN) < 100){
        mp3Basic(CMD_VOLUME_DOWN);
        delay(100);
    } 
}


void intro()
{
  
   setVolume(0X0F); //Set volume to 15 out of 30
   delay(200);
   sendCommand(CMD_SET_VOLUME,0x0F); //sets it
   sendCommand(CMD_PLAY_FILE_NAME,0X0229); // says "Welcome to Button Hero"
   delay(2000);
   sendCommand(CMD_PLAY_FILE_NAME,0X022A); // Plays circus music into
   delay(200);
   cyclePlay(0X022A); //cycles through music
  }

void setVolume(int8_t vol){mp3_5bytes(CMD_SET_VOLUME, vol);}
void playWithVolume(int16_t dat){mp3_6bytes(CMD_PLAY_W_VOL, dat);}

/*cycle play with an index*/
void cyclePlay(int16_t index){mp3_6bytes(CMD_SET_PLAY_MODE,index);}
void setCyleMode(int8_t AllSingle){mp3_5bytes(CMD_SET_PLAY_MODE,AllSingle);}


void playCombine(int8_t song[][2], int8_t number)
{
  if(number > 15) return;//number of songs combined can not be more than 15
  uint8_t nbytes;//the number of bytes of the command with starting byte and ending byte
  nbytes = 2*number + 4;
  int8_t Send_buf[nbytes];
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = nbytes - 2; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = CMD_PLAY_COMBINE; 
  for(uint8_t i=0; i < number; i++)//
  {
    Send_buf[i*2+3] = song[i][0];
  Send_buf[i*2+4] = song[i][1];
  }
  Send_buf[nbytes - 1] = 0xef;
  sendBytes(nbytes);
}


void sendCommand(int8_t command, int16_t dat = 0)
{
  delay(20);
  if((command == CMD_PLAY_W_VOL)||(command == CMD_SET_PLAY_MODE)||(command == CMD_PLAY_COMBINE))
    return;
  else if(command < 0x10) 
  {
  mp3Basic(command);
  }
  else if(command < 0x40)
  { 
  mp3_5bytes(command, dat);
  }
  else if(command < 0x50)
  { 
  mp3_6bytes(command, dat);
  }
  else return;
 
}

void mp3Basic(int8_t command)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x02; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = 0xef; //
  sendBytes(4);
}
void mp3_5bytes(int8_t command, uint8_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x03; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = dat; //
  Send_buf[4] = 0xef; //
  sendBytes(5);
}
void mp3_6bytes(int8_t command, int16_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x04; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = (int8_t)(dat >> 8);//datah
  Send_buf[4] = (int8_t)(dat); //datal
  Send_buf[5] = 0xef; //
  sendBytes(6);
}
void sendBytes(uint8_t nbytes)
{
  for(uint8_t i=0; i < nbytes; i++)//
  {
    myMP3.write(Send_buf[i]) ;
  }
}

//checks and returns the state of button. HIGH is pressed, LOW if not pressed
boolean buttonState(int8_t buttonPin){return digitalRead(buttonPin);}

//lights LEDS according to row seq
void colorRow(unsigned short int colorSeq, unsigned short int delayValue){
  for(unsigned char i = 0; i < 5; i++){
    digitalWrite(LATCH,LOW); //turns off the leds not in the sequence from other colors
    shiftOut(DATA, CLCK, MSBFIRST, colorSeq); //changes to the next light in the color sequence 
    digitalWrite(LATCH,HIGH); //turns on the next light in the sequence
    delay(delayValue); //set the speed the sequence will light through
    
    if(colorSeq == RED && i==4){
       if(buttonState(RB) == HIGH) {
          score++; delay(2);
          display.showNumberDec(score); //Display the Score;
       }//end if
     }//end RED if
     
     if(colorSeq == GREEN && i==4){
       if(buttonState(GB) == HIGH) {
          score++; delay(2);
          display.showNumberDec(score); //Display the Score;
       }//end if
     }//end GREEN if
    
    if(colorSeq == YELLOW && i==4){
       if(buttonState(YB) == HIGH) {
          score++; delay(2);
          display.showNumberDec(score); //Display the Score;
       }//end if
     }//end YELLOW if

     if(colorSeq == BLUE && i==4){
       if(buttonState(BB) == HIGH) {
          score++; delay(100);
          display.showNumberDec(score); //Display the Score;
       }//end if
     }//end BLUE if
     
     if(colorSeq == WHITE && i==4){
       if(buttonState(WB) == HIGH) {
          score++; delay(100);
          display.showNumberDec(score); //Display the Score;
       }//end if
     }//end WHITE if
  }//end for
    
}

