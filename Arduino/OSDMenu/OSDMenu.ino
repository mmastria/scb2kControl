#include <EEPROM.h>
#include <Arduino.h>
#include <ModbusSlave.h>

enum {
  
  MB_UP,
  MB_DOWN,
  MB_LEFT,
  MB_RIGHT,
  MB_SET,
  
  MB_CAM_SELECT,
  
  MB_CAM1_SWITCH,
  MB_CAM2_SWITCH,
  MB_CAM3_SWITCH,

  MB_CAM1_STATUS,
  MB_CAM2_STATUS,
  MB_CAM3_STATUS,
  
  MB_NIGHT_SENSOR,
  MB_RAIN_SENSOR,
  
  MB_NIGHT_RLTYPE,
  MB_RAIN_RLTYPE,
  
  MB_SAVE_SETTINGS,
  
  MB_REGS
};

enum {
  SWITCH_OFF,
  SWITCH_ON,
  SWITCH_AUTO
};

enum {
  POWER_OFF,
  POWER_ON
};

enum {
  CAM1,
  CAM2,
  CAM3,
  CAM_COUNT
};

enum {
  NORMALLY_OPEN,
  NORMALLY_CLOSED
};

#define NIGHT_SENSOR   2

#define CMD_UP         3
#define CMD_DOWN       4
#define CMD_LEFT       5
#define CMD_RIGHT      6
#define CMD_SET        7

#define RAIN_SENSOR    8

#define BUTTON_DOWN    9
#define BUTTON_UP     10
#define BUTTON_LEFT   11
#define BUTTON_SET    12
#define BUTTON_RIGHT  13

#define CMD_CAM1      14 // A0
#define CMD_CAM2      15 // A1
#define CMD_CAM3      16 // A2

#define POWER_CAM1    17 // A3
#define POWER_CAM2    18 // A4
#define POWER_CAM3    19 // A5

#define BUTTON_WAIT_START 5

#define CMD_CAM       CMD_CAM1
#define POWER_CAM     POWER_CAM1

#define EE_SETTED 0
#define EE_CAM_SELECT 1
#define EE_CAM1_SWITCH 2
#define EE_CAM2_SWITCH 3
#define EE_CAM3_SWITCH 4
#define EE_NIGHT_RLTYPE 5
#define EE_RAIN_RLTYPE 6

ModbusSlave mbs;
boolean change=false;
byte buttonWait=0;
byte camera=CAM1;
byte camSwitch[CAM_COUNT];
byte camPower[CAM_COUNT];
byte nightRlType=NORMALLY_OPEN;
byte rainRlType=NORMALLY_CLOSED;
int regs[MB_REGS];

void setup() {
  
  pinMode(CMD_UP, OUTPUT);
  pinMode(CMD_DOWN, OUTPUT);
  pinMode(CMD_LEFT, OUTPUT);
  pinMode(CMD_RIGHT, OUTPUT);
  pinMode(CMD_SET, OUTPUT);

  digitalWrite(CMD_CAM1, LOW);
  digitalWrite(CMD_CAM2, LOW);
  digitalWrite(CMD_CAM3, LOW);

  pinMode(CMD_CAM1, OUTPUT);
  pinMode(CMD_CAM2, OUTPUT);
  pinMode(CMD_CAM3, OUTPUT);

  pinMode(POWER_CAM1, OUTPUT);
  pinMode(POWER_CAM2, OUTPUT);
  pinMode(POWER_CAM3, OUTPUT);

  digitalWrite(POWER_CAM1, HIGH);
  digitalWrite(POWER_CAM2, HIGH); 
  digitalWrite(POWER_CAM3, HIGH); 

  pinMode(NIGHT_SENSOR, INPUT); // external pull-up - Closed = Night (NO)
  pinMode(RAIN_SENSOR, INPUT); // external pull-up - Closed = Not Raining (NC)

  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(BUTTON_SET, INPUT);
  
  // activate pullup resistor
  digitalWrite(BUTTON_UP, HIGH);
  digitalWrite(BUTTON_DOWN, HIGH);
  digitalWrite(BUTTON_LEFT, HIGH);
  digitalWrite(BUTTON_RIGHT, HIGH);
  digitalWrite(BUTTON_SET, HIGH);

  mbs.configure(1, 19200, 'n', 0);
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  for(byte i=0;i<CAM_COUNT;i++)
    camPower[i]=POWER_OFF;  
  readSettings();
  camSelect();
  delay(100);
}

void loop() {
  
  setCamPower();

  // GET BUTTON STATUS
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  if(buttonWait<1) {
    regs[MB_UP]=!digitalRead(BUTTON_UP);
    regs[MB_DOWN]=!digitalRead(BUTTON_DOWN);
    regs[MB_LEFT]=!digitalRead(BUTTON_LEFT);
    regs[MB_RIGHT]=!digitalRead(BUTTON_RIGHT);
    regs[MB_SET]=!digitalRead(BUTTON_SET);
    if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
    if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
    if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
    if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
    if(regs[MB_SET]==1) { change=true; digitalWrite(CMD_SET, HIGH); }
    if(change) {
      change=false;
      buttonWait=BUTTON_WAIT_START;
      delay(50);
      digitalWrite(CMD_UP, LOW);
      digitalWrite(CMD_DOWN, LOW);
      digitalWrite(CMD_LEFT, LOW);
      digitalWrite(CMD_RIGHT, LOW);
      digitalWrite(CMD_SET, LOW);
      delay(100);
    }
  }
  else {
    buttonWait--;
  }
  for(byte i=0; i<MB_REGS; i++)
    regs[i]=0;
  regs[MB_CAM_SELECT]=camera;
  regs[MB_CAM1_SWITCH]=camSwitch[CAM1];
  regs[MB_CAM2_SWITCH]=camSwitch[CAM2];
  regs[MB_CAM3_SWITCH]=camSwitch[CAM3];
  regs[MB_CAM1_STATUS]=camPower[CAM1];
  regs[MB_CAM2_STATUS]=camPower[CAM2];
  regs[MB_CAM3_STATUS]=camPower[CAM3];
  regs[MB_NIGHT_RLTYPE]=nightRlType;
  regs[MB_RAIN_RLTYPE]=rainRlType;

//  regs[MB_NIGHT_SENSOR]=digitalRead(NIGHT_SENSOR);
  regs[MB_NIGHT_SENSOR]=(digitalRead(NIGHT_SENSOR)!=nightRlType ? 1 : 0);
//  regs[MB_RAIN_SENSOR]=!digitalRead(RAIN_SENSOR);
  regs[MB_RAIN_SENSOR]=(digitalRead(RAIN_SENSOR)!=rainRlType ? 1 : 0);

  if(mbs.update(regs, MB_REGS)>4) {
    if(regs[MB_CAM1_SWITCH]!=camSwitch[CAM1]) { camSwitch[CAM1]=(byte)regs[MB_CAM1_SWITCH]; setCamPower(); }
    if(regs[MB_CAM2_SWITCH]!=camSwitch[CAM2]) { camSwitch[CAM2]=(byte)regs[MB_CAM2_SWITCH]; setCamPower(); }
    if(regs[MB_CAM3_SWITCH]!=camSwitch[CAM3]) { camSwitch[CAM3]=(byte)regs[MB_CAM3_SWITCH]; setCamPower(); }
    if(regs[MB_CAM_SELECT]!=camera) { camera=(byte)regs[MB_CAM_SELECT]; camSelect(); }
    if(regs[MB_NIGHT_RLTYPE]!=nightRlType) { nightRlType=(byte)regs[MB_NIGHT_RLTYPE]; setCamPower(); }
    if(regs[MB_RAIN_RLTYPE]!=rainRlType) { rainRlType=(byte)regs[MB_RAIN_RLTYPE]; setCamPower(); }
    if(regs[MB_SAVE_SETTINGS]==1) { saveSettings(); }
    if(regs[MB_UP]==1) { change=true; digitalWrite(CMD_UP, HIGH); }
    if(regs[MB_DOWN]==1) { change=true; digitalWrite(CMD_DOWN, HIGH); }
    if(regs[MB_LEFT]==1) { change=true; digitalWrite(CMD_LEFT, HIGH); }
    if(regs[MB_RIGHT]==1) { change=true; digitalWrite(CMD_RIGHT, HIGH); }
    if(regs[MB_SET]==1) { change=true; digitalWrite(CMD_SET, HIGH); }
    if(change) {
      delay(50);
      digitalWrite(CMD_UP, LOW);
      digitalWrite(CMD_DOWN, LOW);
      digitalWrite(CMD_LEFT, LOW);
      digitalWrite(CMD_RIGHT, LOW);
      digitalWrite(CMD_SET, LOW);
      change=false;
    }
  }
  delay(1);
  
}

void camSelect() {
  digitalWrite(CMD_CAM1, LOW);
  digitalWrite(CMD_CAM2, LOW); 
  digitalWrite(CMD_CAM3, LOW); 
  if(camera==CAM1) {
    digitalWrite(CMD_CAM1, HIGH);
  }
  else {
    if(camera==CAM2) {
      digitalWrite(CMD_CAM2, HIGH); 
    }
    else {
      if(camera==CAM3) {
        digitalWrite(CMD_CAM3, HIGH); 
      }
    }
  }
}

void setCamPower() {
  for(byte i=0; i<CAM_COUNT; i++) {
    switch(camSwitch[i]) {
      case SWITCH_ON:
        camPower[i]=POWER_ON;
        digitalWrite(POWER_CAM+i, LOW);
        break;
      case SWITCH_OFF:
        camPower[i]=POWER_OFF;
        digitalWrite(POWER_CAM+i, HIGH);
        break;
      case SWITCH_AUTO:

//        if(digitalRead(NIGHT_SENSOR)==HIGH and digitalRead(RAIN_SENSOR)==LOW) {
        if(digitalRead(NIGHT_SENSOR)!=nightRlType and digitalRead(RAIN_SENSOR)!=rainRlType) {

          camPower[i]=POWER_ON;
          digitalWrite(POWER_CAM+i, LOW);
        }
        else {
          camPower[i]=POWER_OFF;
          digitalWrite(POWER_CAM+i, HIGH);
        }
        break;
    }
  }
}

void saveSettings() {
  EEPROM.write(EE_SETTED, 1);
  EEPROM.write(EE_CAM_SELECT, camera);
  EEPROM.write(EE_CAM1_SWITCH, camSwitch[CAM1]);
  EEPROM.write(EE_CAM2_SWITCH, camSwitch[CAM2]);
  EEPROM.write(EE_CAM3_SWITCH, camSwitch[CAM3]);
  EEPROM.write(EE_RAIN_RLTYPE, rainRlType);
  EEPROM.write(EE_NIGHT_RLTYPE, nightRlType);
}

void readSettings() {
  if(EEPROM.read(EE_SETTED)==1) {
    camera = EEPROM.read(EE_CAM_SELECT);
    camSwitch[CAM1] = EEPROM.read(EE_CAM1_SWITCH);
    camSwitch[CAM2] = EEPROM.read(EE_CAM2_SWITCH);
    camSwitch[CAM3] = EEPROM.read(EE_CAM3_SWITCH);
    rainRlType = EEPROM.read(EE_RAIN_RLTYPE);
    nightRlType = EEPROM.read(EE_NIGHT_RLTYPE);
    checkSettings();
  }
  else {
    resetSettings();
  }
}

void checkSettings() {
  boolean ok=true;
  if(camera<CAM1 or camera>CAM3) {ok=false;}
  for(byte i=0; i<CAM_COUNT; i++)
    if(camSwitch[i]<SWITCH_OFF or camSwitch[i]>SWITCH_AUTO) {ok=false;}
  if(rainRlType<NORMALLY_OPEN or rainRlType>NORMALLY_CLOSED) {ok=false;}
  if(nightRlType<NORMALLY_OPEN or nightRlType>NORMALLY_CLOSED) {ok=false;}
  if(!ok)
    resetSettings();
}

void resetSettings() {
  camera = CAM1;
  for(byte i=0; i<CAM_COUNT; i++)
    camSwitch[i]=SWITCH_AUTO;
  rainRlType = NORMALLY_CLOSED;
  nightRlType = NORMALLY_OPEN;
}

