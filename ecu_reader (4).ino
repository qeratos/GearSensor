#define RST 9
#define PACKET_BUFFER_SIZE (128)
#define FIRST   2.750
#define SECOND  1.938
#define THIRD   1.556
#define FOUTH   1.348
#define FIFTH   1.208
#define SIXTH   1.095

//#define TEST
//#define SERIAL
#define SLASH Serial.print(" - ");

#include <GyverOLED.h>
#include <SoftwareSerial.h>

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
SoftwareSerial bike(8, 9, false); // RX, TX


uint32_t request_timer = 0;
byte ECU_WAKEUP_MESSAGE[] = {0xFE, 0x04, 0xFF, 0xFF},
                            ECU_INIT_MESSAGE[] = {0x72, 0x05, 0x00, 0xF0, 0x99},
                                test[] = {0x02, 0x04, 0x00, 0xFA},

                                    //data[] = {0x72, 0x07, 0x72, 0x11, 0x00, 0x14, 0xF0}; // 72 07 72 11 00 14 F0
                                    data[] = {0x72, 0x07, 0x72, 0x00, 0x00, 0x14, 0x01};
uint16_t ECU_SUCCESS_CHECKSUM = 0x106;
bool flag = true, conect = false;
struct {

  uint16_t rpm = 0;
  uint8_t speedKPH = 0;

  uint8_t tpsPercent = 0;
  uint8_t tpsVolts;

  float ectTemp;
  uint8_t ectVolts;

  float iatTemp;
  uint8_t iatVolts;

  float battVolts;

  float mapPressure;
  uint8_t mapVolts;
} response;

struct {
  uint8_t engState;
  uint8_t switchState;
} _response;

float calcValueDivide256(int val) {
  return (val * 5) / 256;
}

float calcValueMinus40(int val) {
  return val - 40;
}

float calcValueDivide10(int val) {
  return val / 10;
}

float calcValueDivide16(int val) {
  return (val / 16) * 10;
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
  int initBuffCount = 0;
  byte initBuff[32];
  while ( bike.available() > 0  && initBuffCount < 32 ) {
    initBuff[initBuffCount++] = bike.read();
  }
  int initSum = 0;
  for (int i = 0; i < initBuffCount; i++) {
    initSum += initBuff[i];
  }
#ifdef TEST
  return 1;
#else
  if (initSum == ECU_SUCCESS_CHECKSUM) {
    return 1;// Serial.println("Successfully opened connection to ECU");
  } else {
    return 0; //Serial.println("Failed to open connection to ECU, trying again in 2s");
  }
#endif
}

void request() {
  bike.write(data, sizeof(data));
  delay(50);
  int buffCount = 0;
  byte buff[PACKET_BUFFER_SIZE];
  while ( (bike.available() > 0 )) { // && ( buffCount < PACKET_BUFFER_SIZE)) {
    Serial.print(buff[buffCount]);
    buff[buffCount++] = bike.read();
    //Serial.println(bike.read());
  }
  response.rpm = (uint16_t) buff[5] + buff[6];
  response.speedKPH = buff[18];
  char str[64] = "";
  itoa(response.rpm, str, DEC);
  strcat(str, "*");
  itoa(response.speedKPH, str + strlen(str), DEC);
  strcat(str, "*;");
  Serial.print(str);

#ifdef TEST
  response.tpsPercent = calcValueDivide16(buff[8]);
  response.ectTemp = calcValueMinus40(buff[10]);
  response.iatTemp = calcValueMinus40(buff[12]);
  response.battVolts = calcValueDivide10(buff[17]);
  response.tpsVolts = calcValueDivide256(buff[11]);
  response.mapVolts = calcValueDivide256(buff[13]);
  response.mapPressure = buff[14];
  response.iatVolts = calcValueDivide256(buff[11]);
  response.ectVolts = calcValueDivide256(buff[9]);

#endif



#ifdef SERIAL
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
  if (flag == true) {
    oled.clear();
    flag = false;
  }
  oled.setCursor(32, 2);
  oled.print("RPM:");
  oled.setCursor(70, 2);
  oled.print(response.rpm);

  oled.setCursor(32, 3);
  oled.print("SPEED:");
  oled.setCursor(70, 3);
  oled.print(response.speedKPH);

  oled.setCursor(32, 4);
  oled.print("BATT:");
  oled.setCursor(70, 4);
  oled.print((int)response.battVolts);

  oled.setCursor(32, 5);
  oled.print("TPS:");
  oled.setCursor(70, 5);
  oled.print(response.tpsPercent);

  oled.setCursor(32, 6);
  oled.print("TEMP:");
  oled.setCursor(70, 6);
  oled.print(response.ectTemp);

  oled.update();
}

void send_msg() {

}

void setup() {
  pinMode(PD3, OUTPUT);
  Wire.setClock(800000L);   // макс. 800'000
  Serial.begin(9600);
  bike.begin(10400);
  oled.init();

  oled.clear();
  oled.home();
  oled.setScale(1);

  while (!conect) {
    if (Serial.available() > 0) {
      char key = Serial.read();
      if (key == 'S') {
        Serial.print('A');
        conect = true;
        digitalWrite(PD3, 1);
        delay(50);
        digitalWrite(PD3, 0);
        delay(50);
        digitalWrite(PD3, 1);
        delay(50);
        digitalWrite(PD3, 0);
        delay(50);
      }
    }
  }
}

void loop() {
  if (millis() - request_timer > 1000) {
    request_timer = millis();
    char str[64] = "";
    itoa(request_timer, str, DEC);
    strcat(str, "*");
    itoa(millis(), str + strlen(str), DEC);
    strcat(str, "*;");
    Serial.print(str);


    /*
      if (wake_up()) {
      request();
      print_data();
      send_msg();
      }
    */
  }
}
