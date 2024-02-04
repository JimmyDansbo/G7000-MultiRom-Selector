#include <U8g2lib.h>
#include <EEPROM.h>
#include "romlist.h"

#define MAX_ROMS 32

#define LINE0 13    // Y coordinate for first and second line on the OLED display
#define LINE1 28

#define BUTTON_NOTHING 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_ENTER 3
#define BUTTON_ESC 4      // Currently the ESC key is not used in this code

#define DEBOUNCE_DELAY 50

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); // Initialize display (no rotation)

const byte UpPin = 2;     
const byte DnPin = 3;     
const byte EnterPin = 4; 
const byte EscPin = 10; 

const byte pinA14 = 5;
const byte pinA15 = 6;
const byte pinA16 = 7;
const byte pinA17 = 8;
const byte pinA18 = 9;

byte key = BUTTON_NOTHING;
byte lastkey=BUTTON_NOTHING;
byte keystate=BUTTON_NOTHING;
byte statUp, statDn, statEnter, statEsc;
byte ROMnum;
unsigned long starttime;
char str[MAX_STR_LEN];
bool firstupdate=true;

// Write a byte to 3 different EEPROM locations
// This is to try and ensure that the data can be read
// back without issues
void eWrite(byte data) {
  EEPROM.update(0, data);
  EEPROM.update(4, data);
  EEPROM.update(9, data);
}

// Read a byte from EEPROM
// Function reads from 3 different locations
// To try and ensure that the data read is
// correct.
byte eRead() {
  byte b1, b2, b3;
  b1 = EEPROM.read(0);
  b2 = EEPROM.read(4);
  b3 = EEPROM.read(9);
  if (b1==b2) return (b1);
  if (b1==b3) return (b1);
  if (b2==b3) return (b2);
  return (33);              // Return 33 (error) if no two bytes are the same
}

// Read the buttons and set the key variable accordingly
void check_buttons() {
  statUp = digitalRead(UpPin);
  statDn = digitalRead(DnPin);
  statEnter = digitalRead(EnterPin);
  statEsc = digitalRead(EscPin);

  if (digitalRead(UpPin)==0) key=BUTTON_UP; else
  if (digitalRead(DnPin)==0) key=BUTTON_DOWN; else
  if (digitalRead(EnterPin)==0) key=BUTTON_ENTER; else
  if (digitalRead(EscPin)==0) key=BUTTON_ESC; else
  key=BUTTON_NOTHING;
  delay(1);
}

void updateDisplay(bool cls=false) {
  u8g2.clearBuffer();                                         // Clear the screen
  if (eRead()==ROMnum) u8g2.drawFrame(0,0,128,32);            // Draw a frame if at selected ROM
  sprintf_P(str, PSTR("ROM#: %02d"), ROMnum);           
  u8g2.drawStr(1, LINE0, str);                                // Write current ROMnum
  strcpy_P(str, (char*)pgm_read_word(&(ROMlist[ROMnum])));
  u8g2.drawStr(1, LINE1, str);                                // Write current ROM name
  u8g2.sendBuffer();                                          // Update screen
}

// Read the buttons and act accordingly
void readButtons() {
  check_buttons();

  // Use the DEBOUNCE_DELAY to ensure that a single buttonpress
  // does not register as multiple presses
  if (key != lastkey) starttime=millis();
  if ((millis() - starttime) > DEBOUNCE_DELAY) {
    if (key != keystate) {
      // Butten press has changed...
      keystate=key;
      switch (key) {
        case BUTTON_UP:     ROMnum++;                         // Increment ROM number, but reset to 0 if above MAX_ROMS
                            if (ROMnum==MAX_ROMS) ROMnum=0;
                            break;
        case BUTTON_DOWN:   ROMnum--;                         // Decrement ROM number, but reset to MAX_ROMS if larger than MAX_ROMS
                            if (ROMnum>=MAX_ROMS) ROMnum=MAX_ROMS-1;
                            break;
        case BUTTON_ENTER:  setAddr();                      // Set the Address pins
                            eWrite(ROMnum);                 // Write the new ROM number in EEPROM
                            break;
      }
      updateDisplay();
    }
  }
  lastkey=key;
}

// Set the address pins according to the low 5 bits 
// of the global ROMnum variable
void setAddr() {
  digitalWrite(pinA14, (ROMnum&0b00000001)!=0?HIGH:LOW);
  digitalWrite(pinA15, (ROMnum&0b00000010)!=0?HIGH:LOW);
  digitalWrite(pinA16, (ROMnum&0b00000100)!=0?HIGH:LOW);
  digitalWrite(pinA17, (ROMnum&0b00001000)!=0?HIGH:LOW);
  digitalWrite(pinA18, (ROMnum&0b00010000)!=0?HIGH:LOW);
}

void setup() {
  ROMnum=eRead();
  u8g2.begin();
  pinMode(UpPin, INPUT_PULLUP);
  pinMode(DnPin, INPUT_PULLUP);
  pinMode(EnterPin, INPUT_PULLUP);
  pinMode(EscPin, INPUT_PULLUP);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fur11_tr);
  u8g2.setDrawColor(1);

  if (ROMnum>=33) { //Error in reading from EEPROM
    strcpy_P(str, PSTR("ERR: EEPROM"));
    ROMnum=0;
  } else
    strcpy_P(str, PSTR("by Jimmy Dansbo"));

  u8g2.drawStr(1, LINE0, str);

  setAddr();
  pinMode(pinA14, OUTPUT);
  pinMode(pinA15, OUTPUT);
  pinMode(pinA16, OUTPUT);
  pinMode(pinA17, OUTPUT);
  pinMode(pinA18, OUTPUT);

  strcpy_P(str, PSTR("http://jnz.dk?g7kc"));
  u8g2.drawStr(1, LINE1, str);
  u8g2.sendBuffer();
}

void loop() {
  readButtons();
  if (firstupdate) {
    if (millis()>10000) {
      firstupdate=false;
      updateDisplay();
    }
  }
}
