#include <SoftwareSerial.h>
SoftwareSerial serial(PA6, 0, 0);

#define DELAY 300


class Disp{
/*
     A0
   A1  10
      9
   A2  8
     A3   A7
*/
  public:
  Disp();
  void circle(uint16_t timeout);
  void eght(uint16_t timeout);


  private:
  void draw_bit(byte mask);
  void cls();
};

Disp::Disp(){}

void Disp::circle(uint16_t timeout){
  timeout /= 6;
  this->cls();
  byte bit = 0b00000001;
  while (!(bit & 0b10000000)) {
    if (!(bit & 0b00000010)) {
      this->draw_bit(bit);
      delay(timeout);
      this->cls();
    }
    bit = bit << 1;
  }
}

void Disp::eght(uint16_t timeout){
  timeout /= 9;
  this->draw_bit(0b01000000);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00000001);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00000010);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00010000);  //
  delay(timeout);
  this->cls();

  this->draw_bit(0b00001000);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00000100);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00000010);
  delay(timeout);
  this->cls();

  this->draw_bit(0b00100000);
  delay(timeout);
  this->cls();

  this->draw_bit(0b01000000);
  delay(timeout);
  this->cls();
}

void Disp::draw_bit(byte mask){
  if (mask & 0b00000000) { cls(); }
  if (mask & 0b10000000) { digitalWrite(PA7, HIGH); }
  if (mask & 0b01000000) { digitalWrite(PA0, HIGH); }
  if (mask & 0b00100000) { digitalWrite(PA1, HIGH); }
  if (mask & 0b00010000) { digitalWrite(PA2, HIGH); }
  if (mask & 0b00001000) { digitalWrite(PA3, HIGH); }
  if (mask & 0b00000100) { digitalWrite(8, HIGH); }
  if (mask & 0b00000010) { digitalWrite(9, HIGH); }
  if (mask & 0b00000001) { digitalWrite(10, HIGH); }
}

void Disp::cls(){
  digitalWrite(8, 0);
  digitalWrite(9, 0);
  digitalWrite(PA7, 0);
  digitalWrite(PA3, 0);
  digitalWrite(10, 0);
  digitalWrite(PA0, 0);
  digitalWrite(PA2, 0);
  digitalWrite(PA1, 0);
}

void setup() {
  pinMode(10, OUTPUT);  // RIGHT_UP+
  pinMode(9, OUTPUT);   // CENTR+
  pinMode(8, OUTPUT);   // RIGHT_DOWN+

  pinMode(PA0, OUTPUT);  // UP+
  pinMode(PA1, OUTPUT);  // Left_UP+
  pinMode(PA2, OUTPUT);  // Left_Down+
  pinMode(PA3, OUTPUT);  // DOWN+
  pinMode(PA7, OUTPUT);  // POINT+

  pinMode(PA6, INPUT);
  serial.begin(9600);
  digitalWrite(0, LOW);
}

Disp digi;
void loop() {

  digi.circle(1000);
  digi.eght(1000);
}
