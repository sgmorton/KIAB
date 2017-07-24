/* 
  Menu driven control of a sound board over UART.
  Commands for playing by # or by name (full 11-char name)
  Hard reset and List files (when not playing audio)
  Vol + and - (only when not playing audio)
  Pause, unpause, quit playing (when playing audio)
  Current play time, and bytes remaining & total bytes (when playing audio)

  Connect UG to ground to have the sound board boot into UART mode
*/

#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

// Choose any two pins that can be used with SoftwareSerial to RX & TX  on Arduino board
#define SFX_TX 5
#define SFX_RX 6

// Connect to the RST pin on the Sound Board
#define SFX_RST 4

// piezo attached to analog pin 0 on Arduino board
int knockSensor = 0;               
byte val = 0;
int statePin = LOW;

// eventualy this threshold will be governed by a potentiomter
int THRESHOLD = 100;
int volume = 0;





// You can also monitor the ACT pin for when audio is playing!

// we'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third 
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
// can also try hardware serial with
// Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");
 
  // softwareserial at 9600 baud
  ss.begin(9600);
  // can also do Serial1.begin(9600)
 
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
}


void loop() {
  flushInput();

  val = analogRead(knockSensor);     
  if (val >= THRESHOLD) {
    statePin = !statePin;
    
    //You can adjust volume from 0 (silent) to 204 (loudest) in 2-increments. 
    //To increase the volume, send +\n (plus symbol plus new line). 
    //to decrease volume send -\n (minus symbol plus new line) 
    //and it will reply with the new volume (5 ascii characters, two of which are 
    //going to be zeros and then three digits of volume, plus a new line)

// set volume to 0
      uint16_t v;
      v=sfx.volDown(-204);

      
//set volume to some multiplier of val(ue) of piezo (0-1024)
      volume = val/5;
      v=sfx.volDown(volume);
    
    Serial.println("Kick!");
    if (! sfx.playTrack("kick.ogg") ) {
        Serial.println("Failed to play track?");
      }
  }



  //wait for connection to restablish
  while (!Serial.available());
  char cmd = Serial.read();
  flushInput();
  
  
}






/************************ MENU HELPERS ***************************/

void flushInput() {
  // Read all available serial input to flush pending data.
  uint16_t timeoutloop = 0;
  while (timeoutloop++ < 40) {
    while(ss.available()) {
      ss.read();
      timeoutloop = 0;  // If char was received reset the timer
    }
    delay(1);
  }
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}

uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff) {
  uint16_t buffidx = 0;
  
  while (true) {
    if (buffidx > maxbuff) {
      break;
    }

    if (Serial.available()) {
      char c =  Serial.read();
      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0) {  // the first 0x0A is ignored
          continue;
        }
        buff[buffidx] = 0;  // null term
        return buffidx;
      }
      buff[buffidx] = c;
      buffidx++;
    }
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
/************************ MENU HELPERS ***************************/
