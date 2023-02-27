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
  



void Gdram_Write(uint8_t y, uint8_t x);
uint16_t Gdram_Read();
void SetPixel(uint8_t x, uint8_t y);
void SetPos(uint8_t x, uint8_t y);
void SetPixelPos(uint8_t x, uint8_t y);

void ST_GraphicsON();
void ST_GraphicsOFF();
void ST_WriteCmd(uint8_t cmd);
void ST_Init();
void ST_InitG();
void ST_WriteData(uint8_t dat);
void ST_Epuls();
void ST_SetDataPins(uint8_t val);


const uint8_t data_pins[8] = {13, 12, 14, 27, 26, 25, 33, 32};

const uint8_t index_offset[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

const uint8_t lcd_pozice[64] = {
  0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
  0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
  0x88,0x89,0x90,0x91,0x92,0x93,0x94,0x95,
  0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f
};

enum Barvy{OFF, ON};
uint8_t posX, posY;
Barvy barva = ON;
  


uint16_t gdram[64][8];



void SetPixel(uint8_t x, uint8_t y){
  uint16_t temp = 0;
  Gdram_Write(y, x);
  temp = Gdram_Read();
  SetPosition(x, y);

  uint8_t HByte_temp, LByte_temp;
  HByte_temp = temp >> 8;
  LByte_temp = temp & 0xff;
  //ST_WriteData(temp);
}

void Gdram_Write(uint8_t y, uint8_t x){
  if(x < 0 || x > 127) return;
  if(y <0 || y > 63) return;
  if(barva) gdram[y][x/16] |= (1 << index_offset[x%16]);
  else gdram[y][x/16] &= ~(1 << index_offset[x%16]);
}

uint16_t Gdram_Read(){
  return gdram[posY][posX];
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
  delay(1);
  ST_WriteCmd(EXTENDED_SET);
  delay(1);
  ST_WriteCmd(BASIC_SET);
  delay(1);
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
    delayMicroseconds(10);                                  //nastavit cas nwm
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

  poX = poxY = 0;
  memset(gdram, 0x0000, sizeof(gdram));
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
