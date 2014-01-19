#include <Arduino.h>
#include <ModbusSlave.h>

enum {
  
  MB_UP,      // 0 R/W
  MB_DOWN,    // 1 R/W
  MB_LEFT,    // 2 R/W
  MB_RIGHT,   // 3 R/W
  MB_SELECT,  // 4 R/W

  MB_REGS
};

#define BUTTON_UP      8
#define BUTTON_DOWN    9
#define BUTTON_LEFT   10
#define BUTTON_RIGHT  11
#define BUTTON_SELECT 12

#define CMD_UP     2
#define CMD_DOWN   3
#define CMD_LEFT   4
#define CMD_RIGHT  5
#define CMD_SELECT 6

ModbusSlave mbs;
boolean change=false;
int regs[MB_REGS];

void setup() {

  pinMode(CMD_UP, OUTPUT);
  pinMode(CMD_DOWN, OUTPUT);
  pinMode(CMD_LEFT, OUTPUT);
  pinMode(CMD_RIGHT, OUTPUT);
  pinMode(CMD_SELECT, OUTPUT);
  
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  
  // activate pullup resistor
  digitalWrite(BUTTON_UP, HIGH);
  digitalWrite(BUTTON_DOWN, HIGH);
  digitalWrite(BUTTON_LEFT, HIGH);
  digitalWrite(BUTTON_RIGHT, HIGH);
  digitalWrite(BUTTON_SELECT, HIGH);
  
  mbs.configure(1, 115200, 'n', 0);
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  delay(100);
}

void loop() {
  
  regs[MB_UP]=!digitalRead(BUTTON_UP);
  regs[MB_DOWN]=!digitalRead(BUTTON_DOWN);
  regs[MB_LEFT]=!digitalRead(BUTTON_LEFT);
  regs[MB_RIGHT]=!digitalRead(BUTTON_RIGHT);
  regs[MB_SELECT]=!digitalRead(BUTTON_SELECT);
  
  if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
  if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
  if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
  if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
  if(regs[MB_SELECT]==1) { change=true; digitalWrite(CMD_SELECT, HIGH); }

  if(change) {
    delay(100);
    digitalWrite(CMD_UP, LOW);
    digitalWrite(CMD_DOWN, LOW);
    digitalWrite(CMD_LEFT, LOW);
    digitalWrite(CMD_RIGHT, LOW);
    digitalWrite(CMD_SELECT, LOW);
    change=false;
    for(byte i=0; i<MB_REGS; i++)
      regs[i]=0;
  }

  if(mbs.update(regs, MB_REGS)>4) {
    if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
    if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
    if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
    if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
    if(regs[MB_SELECT]==1) { change=true; digitalWrite(CMD_SELECT, HIGH); }
    
    if(change) {
      delay(100);
      digitalWrite(CMD_UP, LOW);
      digitalWrite(CMD_DOWN, LOW);
      digitalWrite(CMD_LEFT, LOW);
      digitalWrite(CMD_RIGHT, LOW);
      digitalWrite(CMD_SELECT, LOW);
      change=false;
      for(byte i=0; i<MB_REGS; i++)
        regs[i]=0;
    }
  
  }

}

