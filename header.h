#include "Arduino.h"

#define RS  21
#define EN  22
//#define RST 39

#define LCD_CLEAR       0x01
#define LCD_HOME        0x02
#define BASIC_SET       0x30
#define CGRAM_ADR       0x40
#define DDRAM_ADR       0x80
#define SCROLL_RAM_ADR  0x02
#define EXTENDED_SET    0x34
#define GRAPHICS_ON     0x36
#define GDRAM_ADR       0x80
  



void ST_GraphicsON();
void ST_GraphicsOFF();
void ST_WriteCmd(uint8_t cmd);
void ST_Init();
void ST_InitG();
void ST_WriteData(uint8_t dat);
void ST_Epuls();
void ST_SetDataPins(uint8_t val);


const uint8_t data_pins[8] = {13, 12, 14, 27, 26, 25, 33, 32};



void ST_InitG(){
  ST_WriteCmd(0x34);
  delay(1);
  ST_WriteCmd(0x40);
  delay(1);
  ST_WriteCmd(0x01);
  delay(1);
  ST_WriteCmd(0x06);
  delay(1);
  ST_WriteCmd(0x12);
  delayMicroseconds(80);
  delayMicroseconds(80);
  ST_WriteCmd(0x34);
  delay(1);
  ST_WriteCmd(0x02);
  delay(1);
}

void ST_GraphicsON(){
  ST_WriteCmd(0x30);
  delay(1);
  ST_WriteCmd(0x34);
  delay(1);
  ST_WriteCmd(0x36);
  delay(1);
}
void ST_GraphicsOFF(){
  ST_WriteCmd(GRAPHICS_ON);
  ST_WriteCmd(EXTENDED_SET);
  ST_WriteCmd(BASIC_SET);
}

inline void ST_Epuls(){
  //delayMicroseconds(100); 
  digitalWrite(EN,LOW);  
  delayMicroseconds(1);
  digitalWrite(EN,HIGH);  
  delayMicroseconds(1);
  digitalWrite(EN,LOW);    
}

inline void ST_SetDataPins(uint8_t val){
  for(uint8_t i = 0; i < 8; i++)
    digitalWrite(data_pins[i], (val >> i) & 0x01);
    delayMicroseconds(100);
}


inline void ST_WriteCmd(uint8_t cmd){
  digitalWrite(RS, LOW);
  ST_SetDataPins(cmd);
  ST_Epuls();
  delayMicroseconds(100);
}

inline void ST_WriteData(uint8_t dat){
  digitalWrite(RS, HIGH);
  ST_SetDataPins(dat);
  ST_Epuls();
  delayMicroseconds(100);
}

void ST_Init(){
  
  delay(50);   //wait for >40 ms

  for(uint8_t i = 0; i < 8; i++){
    pinMode(data_pins[i], OUTPUT);
  }
  //pinMode(RST, OUTPUT);
  pinMode(RS, OUTPUT);
  pinMode(EN, OUTPUT);
  //****************************
  
  ST_WriteCmd(0x30);  // 8bit mode
  delayMicroseconds(110);  //  >100us delay

  ST_WriteCmd(0x30);  // 8bit mode
  delayMicroseconds(40);  // >37us delay

  ST_WriteCmd(0x08);  // D=0, C=0, B=0
  delayMicroseconds(110);  // >100us delay

  ST_WriteCmd(0x01);  // clear screen
  delay(12);  // >10 ms delay


  ST_WriteCmd(0x06);  // cursor increment right no shift
  delay(1);  // 1ms delay

  ST_WriteCmd(0x0C);  // D=1, C=0, B=0
  delay(1);  // 1ms delay

  ST_WriteCmd(0x02);  // return to home
  delay(1);  // 1ms delay
}
