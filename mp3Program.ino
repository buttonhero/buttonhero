#include <Servo.h>
#include <SPI.h>
// 

/* Wiring Guide */
//Serial MP3 Player A     |  PINS//
//              RX              |   7
//              TX              |   8
//              VCC            |   5V
//              GND            |   GND

#include <SoftwareSerial.h>

#define ARDUINO_RX 8//should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 7//connect to RX of the module
SoftwareSerial myMP3(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*basic commands*/
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F//stop interruptting with a song, just stop the interlude

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

#define CMD_PLAY_COMBINE 0X45//can play combination up to 15 songs

#define SW_PIN 2 // digital pin connected to switch output
#define X_PIN 0 // analog pin connected to X output
#define Y_PIN 1 // analog pin connected to Y output

#define SONGS 5 //number of songs

void sendCommand(int8_t command, int16_t dat );


void setup() 
{

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
int isIntro = 1;
int isPlaying = 1;
int currSong = 1;
void loop() 
{
if(digitalRead(SW_PIN) == LOW){
  if(isIntro == 1){
    playWithVolume(0X0F01);//play song with volume 15(0x0F) class
    isPlaying = 1;
    isIntro = 0;
    delay(1000);    
  }
  else if(isPlaying == 1){
    mp3Basic(CMD_PAUSE);
    isPlaying = 0;
    delay(1000);
  }
  else if(isPlaying == 0) {
    mp3Basic(CMD_PLAY);
    isPlaying = 1;
    delay(1000);
  }  
}


if(analogRead(X_PIN) > 800){
  if(isIntro == 0 )
  {
    if(currSong < SONGS)
    {
      mp3Basic(CMD_NEXT_SONG);
      currSong++;
    }  
    else
    {
    sendCommand(CMD_PLAY_FILE_NAME,0X0101);
    currSong = 1;
    }
  delay(1000);
  }
}
if(analogRead(X_PIN) < 200){
  if(isIntro == 0 )
  {
  if(currSong > 1)
  {
    mp3Basic(CMD_PREV_SONG);
    currSong--;
  }  
  else
  {
    sendCommand(CMD_PLAY_FILE_NAME,0X0105);
    currSong = SONGS;
  }
  delay(1000);
  }
}
if(analogRead(Y_PIN) > 800){
  mp3Basic(CMD_VOLUME_UP);
  delay(100);
}
if(analogRead(Y_PIN) < 200){
  mp3Basic(CMD_VOLUME_DOWN);
  delay(100);
}

}

void intro()
{
  
  setVolume(0X0F);
  delay(200);
  //sendCommand(CMD_SET_VOLUME,0x0F);
  sendCommand(CMD_PLAY_FILE_NAME,0X0229);
  delay(2000);
  setVolume(0X0F);
  delay(200);
  sendCommand(CMD_PLAY_FILE_NAME,0X022A);
  delay(200);
  cyclePlay(0X022A);
}

void setVolume(int8_t vol)
{
  mp3_5bytes(CMD_SET_VOLUME, vol);
}
void playWithVolume(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_W_VOL, dat);
}

/*cycle play with an index*/
void cyclePlay(int16_t index)
{
  mp3_6bytes(CMD_SET_PLAY_MODE,index);
}

void setCyleMode(int8_t AllSingle)
{
  mp3_5bytes(CMD_SET_PLAY_MODE,AllSingle);
}


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


