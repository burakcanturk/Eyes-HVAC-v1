#include <EEPROM.h>
#include <IR_Remote.h>
#include <HLK_LD2450.h>
#include <STM32LowPower.h>

//HardwareSerial ser(USART1);
HardwareSerial hlk_ser(USART2);

//#define CONSOLE_SERIAL ser

#define NORMAL_MODE 0
#define NEW_DATA_MODE 1
#define SPEED_SET_MODE 2
#define IR_TEST_MODE 3

#define ir_recv_pin PB11
#define ir_send_pin PB10

#define button_pin PC14

#define buzzer_pin PC15

#define rgb_red_pin PB12
#define rgb_green_pin PB13
#define rgb_blue_pin PB14

#define minute_5_pin PB3
#define minute_10_pin PB4
#define minute_15_pin PB5
#define minute_20_pin PB6
#define minute_25_pin PB7
#define minute_30_pin PB8

#define hlk_open_pin PA1

#define ir_test_button_pin PA7

#define minute_factor 60

#define SECOND_5_TIME 5000
#define SECOND_10_TIME 10000
#define SECOND_15_TIME 15000
#define SECOND_20_TIME 20000
#define SECOND_25_TIME 25000
#define SECOND_30_TIME 30000

IRRecv irrecv(ir_recv_pin);
IRSend irsend(ir_send_pin);
HLK_LD2450 ld2450(&hlk_ser);

uint16_t air_cond_vals[256] = {0};
uint16_t air_cond_vals_len;

bool is_interrupt = false;

unsigned long delay_time = SECOND_5_TIME;

void clearEEPROM();

void setRgb(bool r, bool g, bool b);

char mode = 0;

void humanControl();
void newRCRecording();
void setMinute();

void RCRecordingMode() {
  mode = NEW_DATA_MODE;
}

void setMinuteMode() {
  mode = SPEED_SET_MODE;
}

void irTestMode() {
  mode = IR_TEST_MODE;
}

void setup() {

  //CONSOLE_SERIAL.println("Sistem baslatiliyor...");

  pinMode(hlk_open_pin, OUTPUT);

  digitalWrite(hlk_open_pin, LOW);
  //digitalWrite(hlk_open_pin, HIGH);

  //clearEEPROM();

  hlk_ser.begin(256000);

  //CONSOLE_SERIAL.begin(115200);

  irrecv.begin();
  irsend.begin();

  ld2450.begin();

  pinMode(button_pin, INPUT_PULLUP);

  pinMode(buzzer_pin, OUTPUT);

  pinMode(rgb_red_pin, OUTPUT);
  pinMode(rgb_green_pin, OUTPUT);
  pinMode(rgb_blue_pin, OUTPUT);

  pinMode(minute_5_pin, INPUT_PULLUP);
  pinMode(minute_10_pin, INPUT_PULLUP);
  pinMode(minute_15_pin, INPUT_PULLUP);
  pinMode(minute_20_pin, INPUT_PULLUP);
  pinMode(minute_25_pin, INPUT_PULLUP);
  pinMode(minute_30_pin, INPUT_PULLUP);

  pinMode(ir_test_button_pin, INPUT_PULLUP);

  /*attachInterrupt(digitalPinToInterrupt(minute_5_pin), minute5Ayarla, FALLING);
    attachInterrupt(digitalPinToInterrupt(minute_10_pin), minute10Ayarla, FALLING);
    attachInterrupt(digitalPinToInterrupt(minute_15_pin), minute15Ayarla, FALLING);
    attachInterrupt(digitalPinToInterrupt(minute_20_pin), minute20Ayarla, FALLING);*/

  setRgb(1, 0, 0);

  uint8_t air_cond_vals_len_split[2] = {EEPROM.read(0), EEPROM.read(1)};
  memcpy(&air_cond_vals_len, air_cond_vals_len_split, 2);
  air_cond_vals_len /= 2;

  if (air_cond_vals_len > 0) {
    for (int i = 0; i < air_cond_vals_len * 2; i += 2) {
      uint16_t vals_connect;
      uint8_t vals_split[2] = {EEPROM.read(i + 2), EEPROM.read(i + 3)};
      memcpy(&vals_connect, vals_split, 2);
      air_cond_vals[i / 2] = vals_connect;
    }
  }

  else newRCRecording();

  LowPower.begin();

  LowPower.attachInterruptWakeup(button_pin, RCRecordingMode, FALLING, SLEEP_MODE);

  LowPower.attachInterruptWakeup(minute_5_pin, setMinuteMode, CHANGE, SLEEP_MODE);
  LowPower.attachInterruptWakeup(minute_10_pin, setMinuteMode, CHANGE, SLEEP_MODE);
  LowPower.attachInterruptWakeup(minute_15_pin, setMinuteMode, CHANGE, SLEEP_MODE);
  LowPower.attachInterruptWakeup(minute_20_pin, setMinuteMode, CHANGE, SLEEP_MODE);
  LowPower.attachInterruptWakeup(minute_25_pin, setMinuteMode, CHANGE, SLEEP_MODE);
  LowPower.attachInterruptWakeup(minute_30_pin, setMinuteMode, CHANGE, SLEEP_MODE);

  LowPower.attachInterruptWakeup(ir_test_button_pin, irTestMode, FALLING, SLEEP_MODE);

  for (int i = 0; i < 2; i++) {
    digitalWrite(buzzer_pin, HIGH);
    delay(100);
    digitalWrite(buzzer_pin, LOW);
    delay(100);
  }

check_again_start:

  bool minute_5_val = digitalRead(minute_5_pin);
  bool minute_10_val = digitalRead(minute_10_pin);
  bool minute_15_val = digitalRead(minute_15_pin);
  bool minute_20_val = digitalRead(minute_20_pin);
  bool minute_25_val = digitalRead(minute_25_pin);
  bool minute_30_val = digitalRead(minute_30_pin);

  if (not minute_5_val and
      minute_10_val and
      minute_15_val and
      minute_20_val and
      minute_25_val and
      minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_5_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 5 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           not minute_10_val and
           minute_15_val and
           minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_10_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 10 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           not minute_15_val and
           minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_15_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 15 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           not minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_20_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 20 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           minute_20_val and
           not minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_25_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 25 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           minute_20_val and
           minute_25_val and
           not minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_30_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 30 minuteya ayarlanmistir.");
  }

  bool minute_5_val_next = digitalRead(minute_5_pin);
  bool minute_10_val_next = digitalRead(minute_10_pin);
  bool minute_15_val_next = digitalRead(minute_15_pin);
  bool minute_20_val_next = digitalRead(minute_20_pin);
  bool minute_25_val_next = digitalRead(minute_25_pin);
  bool minute_30_val_next = digitalRead(minute_30_pin);

  if (minute_5_val_next != minute_5_val or
      minute_10_val_next != minute_10_val or
      minute_15_val_next != minute_15_val or
      minute_20_val_next != minute_20_val or
      minute_25_val_next != minute_25_val or
      minute_30_val_next != minute_30_val) {
    //setMinute();
    goto check_again_start;
  }
}

void loop() {

  LowPower.deepSleep(delay_time);

  switch (mode) {
    case NORMAL_MODE:
      humanControl();
      break;
    case NEW_DATA_MODE:
      newRCRecording();
      break;
    case SPEED_SET_MODE:
      setMinute();
      break;
    case IR_TEST_MODE:
      irTest();
      break;
  }

  //LowPower.deepSleep(delay_time);
}

void clearEEPROM() {
  for (int i = 0; i < EEPROM.length(); i++) EEPROM.write(i, 0);
}

void setRgb(bool r, bool g, bool b) {
  digitalWrite(rgb_red_pin, r);
  digitalWrite(rgb_green_pin, g);
  digitalWrite(rgb_blue_pin, b);
}

void humanControl() {

  setRgb(0, 0, 0);

  digitalWrite(hlk_open_pin, HIGH);

  for (int i = 0; i < 30; i++) {

    ld2450.read();

    if (ld2450.getTargetX() != 0 or
        ld2450.getTargetY() != -32768 or
        ld2450.getSpeed() != 0 or
        ld2450.getDistanceResolution() != 0) {
      //irsend.sendRaw(air_cond_vals, air_cond_vals_len, 50);
      setRgb(0, 1, 0);
      //CONSOLE_SERIAL.println("Klima acildi...");
      break;
    }
  }

  digitalWrite(hlk_open_pin, LOW);

  //CONSOLE_SERIAL.print("Target X: ");
  //CONSOLE_SERIAL.println(ld2450.getTargetX());
  //CONSOLE_SERIAL.print("Target Y: ");
  //CONSOLE_SERIAL.println(ld2450.getTargetY());
  //CONSOLE_SERIAL.print("Speed: ");
  //CONSOLE_SERIAL.println(ld2450.getSpeed());
  //CONSOLE_SERIAL.print("tDisRes: ");
  //CONSOLE_SERIAL.println(ld2450.getDistanceResolution());
  //CONSOLE_SERIAL.println("-----------------------------------------------");

  //setRgb(0, 0, 0);
  //delay(50);

  if (ld2450.getTargetX() == 0 and
      ld2450.getTargetY() == -32768 and
      ld2450.getSpeed() == 0 and
      ld2450.getDistanceResolution() == 0) {

    digitalWrite(hlk_open_pin, HIGH);

    for (int i = 0; i < 30; i++) {

      ld2450.read();

      if (ld2450.getTargetX() != 0 or
          ld2450.getTargetY() != -32768 or
          ld2450.getSpeed() != 0 or
          ld2450.getDistanceResolution() != 0) {
        //irsend.sendRaw(air_cond_vals, air_cond_vals_len, 50);
        setRgb(0, 1, 0);
        //CONSOLE_SERIAL.println("Klima acildi...");
        break;
      }
    }

    digitalWrite(hlk_open_pin, LOW);

    irsend.sendRaw(air_cond_vals, air_cond_vals_len, 50);
    setRgb(1, 0, 0);
    //CONSOLE_SERIAL.println("Klima kapandi!!!");
  }
}

void newRCRecording() {

  setRgb(1, 1, 0);

  digitalWrite(buzzer_pin, HIGH);
  delay(750);
  digitalWrite(buzzer_pin, LOW);

  //CONSOLE_SERIAL.println("Yeni klima kapatma sinyalini gonderiniz...");

  uint16_t vals[256];
  int len = irrecv.recvRaw(vals, 30000);
  while (len < 32 and  len != -1) len = irrecv.recvRaw(vals, 30000);

  if (len != -1) {

    setRgb(1, 0, 1);

    digitalWrite(buzzer_pin, HIGH);
    delay(100);
    digitalWrite(buzzer_pin, LOW);

    air_cond_vals_len = len;
    memcpy(air_cond_vals, vals, len * 2);

    //CONSOLE_SERIAL.print("Deger sayisi: ");
    //CONSOLE_SERIAL.println(air_cond_vals_len);

    uint8_t air_cond_vals_len_split[2];
    air_cond_vals_len *= 2;
    memcpy(air_cond_vals_len_split, &air_cond_vals_len, 2);
    air_cond_vals_len /= 2;
    EEPROM.write(0, air_cond_vals_len_split[0]);
    EEPROM.write(1, air_cond_vals_len_split[1]);

    //CONSOLE_SERIAL.print("Yeni degerler: ");
    for (int i = 0; i < air_cond_vals_len * 2; i += 2) {
      //CONSOLE_SERIAL.print(air_cond_vals[i / 2]);
      //CONSOLE_SERIAL.print(" ");
      byte vals_split[2];
      memcpy(vals_split, air_cond_vals + i / 2, 2);
      EEPROM.write(i + 2, vals_split[0]);
      EEPROM.write(i + 3, vals_split[1]);
    }

    setRgb(0, 1, 1);

    for (int i = 0; i < 3; i++) {
      digitalWrite(buzzer_pin, HIGH);
      delay(100);
      digitalWrite(buzzer_pin, LOW);
      delay(100);
    }

    //CONSOLE_SERIAL.println();
    //CONSOLE_SERIAL.println("Yeni klima kapatma sinyaliniz kaydedilmistir.");

    setRgb(1, 0, 0);
  }

  else {
    setRgb(0, 0, 1);
    digitalWrite(buzzer_pin, HIGH);
    delay(1000);
    digitalWrite(buzzer_pin, LOW);
    //CONSOLE_SERIAL.println("Kaydetme islemi iptal edildi!!!");
    setRgb(1, 0, 0);
  }

  mode = NORMAL_MODE;
}

void setMinute() {

  /*digitalWrite(buzzer_pin, HIGH);
    delay(500);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_5_TIME;
    //CONSOLE_SERIAL.println("TIME 5 minuteya ayarlanmistir.");
    mode = NORMAL_MODE;*/

  //delay(500);

new_control_func:

  bool minute_5_val = digitalRead(minute_5_pin);
  bool minute_10_val = digitalRead(minute_10_pin);
  bool minute_15_val = digitalRead(minute_15_pin);
  bool minute_20_val = digitalRead(minute_20_pin);
  bool minute_25_val = digitalRead(minute_25_pin);
  bool minute_30_val = digitalRead(minute_30_pin);

  if (not minute_5_val and
      minute_10_val and
      minute_15_val and
      minute_20_val and
      minute_25_val and
      minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_5_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 5 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           not minute_10_val and
           minute_15_val and
           minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_10_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 10 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           not minute_15_val and
           minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_15_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 15 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           not minute_20_val and
           minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_20_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 20 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           minute_20_val and
           not minute_25_val and
           minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_25_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 25 minuteya ayarlanmistir.");
  }

  else if (minute_5_val and
           minute_10_val and
           minute_15_val and
           minute_20_val and
           minute_25_val and
           not minute_30_val) {
    digitalWrite(buzzer_pin, HIGH);
    delay(250);
    digitalWrite(buzzer_pin, LOW);
    delay_time = SECOND_30_TIME * minute_factor;
    //CONSOLE_SERIAL.println("TIME 30 minuteya ayarlanmistir.");
  }

  bool minute_5_val_next = digitalRead(minute_5_pin);
  bool minute_10_val_next = digitalRead(minute_10_pin);
  bool minute_15_val_next = digitalRead(minute_15_pin);
  bool minute_20_val_next = digitalRead(minute_20_pin);
  bool minute_25_val_next = digitalRead(minute_25_pin);
  bool minute_30_val_next = digitalRead(minute_30_pin);

  if (minute_5_val_next != minute_5_val or
      minute_10_val_next != minute_10_val or
      minute_15_val_next != minute_15_val or
      minute_20_val_next != minute_20_val or
      minute_25_val_next != minute_25_val or
      minute_30_val_next != minute_30_val) {
    //setMinute();
    goto new_control_func;
  }

  mode = NORMAL_MODE;
}

void irTest() {
  irsend.sendRaw(air_cond_vals, air_cond_vals_len, 50);
  mode = NORMAL_MODE;
}
