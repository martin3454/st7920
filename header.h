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

#define ABS(x) ((x < 0) ? -x : x)


void Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void circle_points(uint8_t x, uint8_t y, uint8_t xc, uint8_t yc);
void Circle(uint8_t xc, uint8_t yc, uint8_t r);
void Gdram_Write(uint8_t y, uint8_t x);
uint16_t Gdram_Read();
void SetPixel(uint8_t x, uint8_t y);
uint8_t CursorPos(uint8_t x, uint8_t y);
void SetCursor(uint8_t x, uint8_t y);
void SetPosition(uint8_t x, uint8_t y);

void ClearScreen();
void FillScreen();



void ST_GraphicsON();
void ST_GraphicsOFF();
void ST_WriteCmd(uint8_t cmd);
void ST_Init();
void ST_WriteData(uint8_t dat);
void ST_Epuls();
void ST_SetDataPins(uint8_t val);


const uint8_t data_pins[8] = {13, 12, 14, 27, 26, 25, 33, 32};

const uint8_t index_offset[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

const uint8_t lcd_pozice[4][16] = {
  {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f},
  {0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f},
  {0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf},
  {0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf}
};

enum Barvy{OFF, ON};
uint8_t posX, posY;
Barvy barva = ON;
uint16_t gdram[64][8];


void Circle(uint8_t xc, uint8_t yc, uint8_t r){

  int pk, x, y;
  pk = 3 - 2 * r;
  x = 0;
  y = r;
  circle_points(x, y, xc, yc);
  while(x < y){
    if(pk < 0){
      pk = pk + 4*x + 6;
      circle_points(++x, y, xc, yc);
    }else{
      pk = pk + 4 * (x - y) + 10;
      circle_points(++x, --y, xc, yc);
    }
  }
}

void circle_points(uint8_t x, uint8_t y, uint8_t xc, uint8_t yc){

  SetPixel(x + xc, y + yc);
  SetPixel(-x + xc, y + yc);
  SetPixel(x + xc, -y + yc);
  SetPixel(-x + xc, -y + yc);
  SetPixel(y + xc, x + yc);
  SetPixel(y + xc, -x + yc);
  SetPixel(-y + xc, x + yc);
  SetPixel(-y + xc, -x + yc);
}


void Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
  int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
  dx = x2 - x1;
  dy = y2 - y1;
  dx1 = ABS(dx);
  dy1 = ABS(dy);
  px = 2*dy1 - dx1;
  py = 2*dx1 - dy1;

  if(dy1 <= dx1){
    if(dx >= 0){
      x = x1;
      y = y1;
      xe = x2;
    }else{
      x = x2;
      y = y2;
      xe = x1;
    }
    SetPixel(x,y);
    while(x < xe){
      x = x + 1;
      if(px < 0) px = px + 2*dy1;
      else{
        if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1;
        else y = y - 1;
        px = px + 2*(dy1 - dx1);
      }
      SetPixel(x,y);
    }
  }else{
    if(dy >= 0){
      x = x1;
      y = y1;
      ye = y2;
    }else{
      x = x2;
      y = y2;
      ye = y1;
    }
    SetPixel(x,y);
    while(y < ye){
      y = y + 1;
      if(py <= 0) py = py + 2*dx1;
      else{
        if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1;
        else x = x - 1;
        py = py + 2*(dx1 - dy1);
      }
      SetPixel(x, y);
    }
  }
}



void ClearScreen(){
  memset(gdram, 0x0000, sizeof(gdram));
  for (byte x = 0; x < 16; x++)
    for (byte y = 0; y < 32; y++) {
      ST_WriteCmd(GDRAM_ADR | y);
      ST_WriteCmd(GDRAM_ADR | x);
      ST_WriteData(0x00);
      ST_WriteData(0x00);
    }
}
void FillScreen(){
  memset(gdram, 0xffff, sizeof(gdram));
  for (byte x = 0; x < 16; x++)
    for (byte y = 0; y < 32; y++) {
      ST_WriteCmd(GDRAM_ADR | y);
      ST_WriteCmd(GDRAM_ADR | x);
      ST_WriteData(0xff);
      ST_WriteData(0xff);
    }
}


void SetCursor(uint8_t x, uint8_t y){
  if(x < 0 || x > 15) return;
  if(y < 0 || y > 4) return;

  uint8_t pos = CursorPos(x,y);
  ST_WriteCmd(pos);
}


uint8_t CursorPos(uint8_t x, uint8_t y){
  return lcd_pozice[y][x];
}

void SetPosition(uint8_t x, uint8_t y){
  //1. vertikal(6-0) 2. horizontal{3-0) 
  if(y < 0 || y > 63) return;
  if(x < 0 || x > 127) return;

  posX = x / 16;
  if(y > 31) posX |= 0x08;
  
  posY = y % 32;
  ST_WriteCmd(GDRAM_ADR | posY);
  
  ST_WriteCmd(GDRAM_ADR | posX);
  posX &= 0x07;
  posY = y;
}

void SetPixel(uint8_t x, uint8_t y){
  uint16_t temp = 0;
  Gdram_Write(y, x);
  SetPosition(x, y);
  temp = Gdram_Read();
  uint8_t temp_hbyte, temp_lbyte;
  temp_hbyte = temp >> 8;
  temp_lbyte = temp & 0xff;
  ST_WriteData(temp_hbyte);
  ST_WriteData(temp_lbyte);
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
  digitalWrite(EN,LOW);  
  delayMicroseconds(1);
  digitalWrite(EN,HIGH);  
  delayMicroseconds(1);
  digitalWrite(EN,LOW);    
}

inline void ST_SetDataPins(uint8_t val){
  for(uint8_t i = 0; i < 8; i++)
    digitalWrite(data_pins[i], (val >> i) & 0x01);
    //delayMicroseconds(10);                                  //nastavit cas nwm
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

  posX = posY = 0;
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
