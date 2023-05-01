#define RST 9
#define PACKET_BUFFER_SIZE (128)
#define FIRST 2.750
#define SECOND 1.938
#define THIRD 1.556
#define FOUTH 1.348
#define FIFTH 1.208
#define SIXTH 1.095

#define TEST
//#define SERIAL
#define SLASH Serial.print(" - ");

#include <GyverOLED.h>
#include <SoftwareSerial.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled(0x3C);
SoftwareSerial bike(8, 9, false);  // RX, TX


uint32_t request_timer = 0;
byte ECU_WAKEUP_MESSAGE[] = { 0xFE, 0x04, 0xFF, 0xFF },
     ECU_INIT_MESSAGE[] = { 0x72, 0x05, 0x00, 0xF0, 0x99 },
     test[] = { 0x02, 0x04, 0x00, 0xFA },
     data[] = { 0x72, 0x07, 0x72, 0x11, 0x00, 0x14, 0xF0 };  // 72 07 72 11 00 14 F0
uint16_t ECU_SUCCESS_CHECKSUM = 0x106;
bool flag = true, conect = false;

float k = 0;
byte gear = 0;

struct {

  uint16_t rpm = 0;
  uint8_t speedKPH = 0;

  uint8_t tpsPercent = 0;
  uint8_t tpsVolts = 0;

  uint8_t ectTemp = 0;
  uint8_t ectVolts = 0;

  uint8_t iatTemp = 0;
  uint8_t iatVolts = 0;

  float battVolts = 0;

  float mapPressure = 0;
  uint8_t mapVolts = 0;
} response;

struct {
  uint8_t engState;
  uint8_t switchState;
} _response;

float calcValueDivide256(int val) {
  return (val * 5.0) / 256.0;
}

float calcValueMinus40(int val) {
  return val - 40.0;
}

float calcValueDivide10(float val) {
  return val / 10.0;
}

float calcValueDivide16(int val) {
  return (val / 16.0) * 10.0;
}

float calcKPHtoMPH(int val) {
  return val / 1.609344;
}


uint8_t calcChecksum(const uint8_t* data, uint8_t len) {
  uint8_t cksum = 0;
  for (uint8_t i = 0; i < len; i++) {
    cksum += data[i];
  }
  oled.clear();
  oled.home();
  oled.print(cksum);
  oled.update();
  delay(2000);
  return cksum;
}

int wake_up() {
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(70);
  digitalWrite(RST, HIGH);
  delay(120);
  bike.write(ECU_WAKEUP_MESSAGE, sizeof(ECU_WAKEUP_MESSAGE));
  delay(200);
  bike.write(ECU_INIT_MESSAGE, sizeof(ECU_INIT_MESSAGE));
  bike.flush();
  delay(50);

  /*
  int initBuffCount = 0;
  byte initBuff[32];
  while (bike.available() > 0 && initBuffCount < 32) {
    initBuff[initBuffCount++] = bike.read();
  }
  int initSum = 0;
  for (int i = 0; i < initBuffCount; i++) {
    initSum += initBuff[i];
  }
  */
}



void request() {
  bike.write(data, sizeof(data));
  delay(50);
  int buffCount = 0;
  byte buff[PACKET_BUFFER_SIZE];
  while ((bike.available() > 0) && ( buffCount < PACKET_BUFFER_SIZE)) {
    buff[buffCount++] = bike.read();
  }

  response.speedKPH = buff[18];
  response.ectTemp = calcValueMinus40(buff[15]);
  response.rpm = uint16_t(buff[10] << 8)+buff[11];
  response.battVolts = calcValueDivide10(buff[22]);
  response.speedKPH = buff[23];
  response.tpsPercent = calcValueDivide16(buff[13]);
  response.iatTemp = calcValueMinus40(buff[17]);

  k = response.speedKPH / 0.0006 / 276.0 * 8.0;
  k = response.rpm / k;
  if(k >= 2.5 && k <= 2.9){
    gear = 1;
  } else if (k >= 1.7 && k <= 2.1){
    gear = 2;
  } else if (k >= 1.4 && k <= 1.7){
    gear = 3;    
  } else if (k >= 1.25 && k <= 1.39){
    gear = 4;
  } else if (k >= 1.15 && k <= 1.24){
    gear = 5;
  } else if (k >= 0.9 && k <= 1.14){
    gear = 6;
  }else{
    gear = 0;
  }


#ifdef ALL
  response.tpsPercent = calcValueDivide16(buff[8]);
  response.ectTemp = calcValueMinus40(buff[10]);
  response.iatTemp = calcValueMinus40(buff[12]);
  response.battVolts = calcValueDivide10(buff[17]);
  response.tpsVolts = calcValueDivide256(buff[11]);
  response.mapVolts = calcValueDivide256(buff[13]);
  response.mapPressure = buff[14];
  response.iatVolts = calcValueDivide256(buff[11]);
  response.ectVolts = calcValueDivide256(buff[9]);

  Serial.print("RPM: ");
  Serial.print(response.rpm);
  SLASH
  Serial.print("KPH: ");
  Serial.print(response.speedKPH);
  SLASH
  Serial.print("TPS: ");
  Serial.print(response.tpsPercent);
  SLASH
  Serial.print("tps: ");
  Serial.print(response.tpsVolts);
  SLASH
  Serial.print("ECT: ");
  Serial.print(response.ectTemp);
  SLASH
  Serial.print("ect: ");
  Serial.println(response.ectVolts);
  SLASH
  Serial.print("IAT: ");
  Serial.print(response.iatTemp);
  SLASH
  Serial.print("iat: ");
  Serial.print(response.iatVolts);
  SLASH
  Serial.print("BATT: ");
  Serial.print(response.battVolts);
  SLASH
  Serial.print("MAP: ");
  Serial.print(response.mapPressure);
  SLASH
  Serial.print("map: ");
  Serial.print(response.mapVolts);

#endif
}

void print_data() {
  //oled.clear();
  oled.setScale(2);
  oled.setCursor(0, 0);
  oled.print("RPM:");
  oled.setCursor(50, 0);
  oled.print(response.rpm);

  oled.setCursor(0, 2);
  oled.print("SPD:");
  oled.setCursor(50, 2);
  oled.print(response.speedKPH);

  /*
  oled.setScale(1);
  oled.setCursor(0, 4);
  oled.print("BATT:");
  oled.setCursor(38, 4);
  oled.print(response.battVolts);

  oled.setCursor(0, 5);
  oled.print("TEMP:");
  oled.setCursor(38, 5);
  oled.print(response.ectTemp);
  */

  oled.setCursor(0, 4);
  oled.print("K:");
  oled.setCursor(23, 4);
  oled.print(k, 2);

  oled.setCursor(0, 6);
  oled.print("T:");
  oled.print(23, 6);
  if(response.iatTemp > 0){
    oled.print(response.iatTemp);
  }
  oled.setScale(4);
  
  oled.setCursor(95, 4);
  if(gear == 0){
    oled.print('N');
  }else{
    oled.print(gear);
  }
  oled.update();
}

void setup() {
  pinMode(PD3, OUTPUT);
  Serial.begin(9600);
  bike.begin(10400);
  
  oled.init();
  oled.clear(); 
  oled.home(); 
  oled.setScale(3);
  oled.flipH(true);
  oled.flipV(true);
  oled.println("DUMPER");
  oled.println("v1.4");
  oled.setScale(1);
  oled.setCursor(50, 5);
  oled.print(__DATE__);


  
  delay(1000);
  oled.clear();
  oled.setScale(2);
}

void loop() {
  if (millis() - request_timer > 500) {
    request_timer = millis();
    digitalWrite(PD3, LOW);
    wake_up();
    request();
    print_data();
  }
}
