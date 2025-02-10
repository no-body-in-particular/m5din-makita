#include <EEPROM.h>
#include <OneWire2.h>
#include "M5DinMeter.h"

OneWire makita(1);
#define ENABLEPIN 2

#define SWAP_NIBBLES(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
#define TURNOFF_SECONDS 60


void cmd_and_read_33(byte *cmd, uint8_t cmd_len, byte *rsp, uint8_t rsp_len,
                     bool off = true) {
  int i;
  digitalWrite(ENABLEPIN, HIGH);
  delay(800);

  makita.reset();
  delayMicroseconds(400);
  makita.write(0x33, 0);

  for (i = 0; i < 8; i++) {
    delayMicroseconds(90);
    rsp[i] = makita.read();
  }

  for (i = 0; i < cmd_len; i++) {
    delayMicroseconds(90);
    makita.write(cmd[i], 0);
  }

  for (i = 8; i < rsp_len + 8; i++) {
    delayMicroseconds(90);
    rsp[i] = makita.read();
  }

  if (off) {
    digitalWrite(ENABLEPIN, LOW);
    delay(400);
  }
}

void cmd_and_read_cc(byte *cmd, uint8_t cmd_len, byte *rsp, uint8_t rsp_len) {
  int i;
  digitalWrite(ENABLEPIN, HIGH);
  delay(800);

  makita.reset();
  delayMicroseconds(400);
  makita.write(0xcc, 0);

  for (i = 0; i < cmd_len; i++) {
    delayMicroseconds(90);
    makita.write(cmd[i], 0);
  }

  for (i = 0; i < rsp_len; i++) {
    delayMicroseconds(90);
    rsp[i] = makita.read();
  }

  digitalWrite(ENABLEPIN, LOW);
  delay(400);
}

void model_cmd(byte rsp[]) {
  byte cmd_params[] = {0xDC, 0x0C};
  cmd_and_read_cc(cmd_params, 2, rsp, 10);
}

void read_data_request(byte rsp[]) {
  byte cmd_params[] = {0xD7, 0x00, 0x00, 0xFF};
  cmd_and_read_cc(cmd_params, 4, rsp, 29);
}

void testmode_cmd() {
  byte cmd_params[] = {0xD9, 0x96, 0xA5};
  byte rsp[64];
  memset(rsp, '\0', 64);

  cmd_and_read_33(cmd_params, 3, rsp, 29, false);
}

void leds_on_cmd() {
  byte cmd_params[] = {0xDA, 0x31};
  byte rsp[32];
  memset(rsp, '\0', 32);
  cmd_and_read_33(cmd_params, 2, rsp, 9);
}

void leds_off_cmd() {
  byte cmd_params[] = {0xDA, 0x34};
  byte rsp[32];
  memset(rsp, '\0', 32);
  cmd_and_read_33(cmd_params, 2, rsp, 9);
}

void reset_error_cmd() {
  byte cmd_params[] = {0xDA, 0x04};
  byte rsp[32];
  memset(rsp, '\0', 32);
  cmd_and_read_33(cmd_params, 2, rsp, 9);
}

void romid_charger_cmd(byte rsp[]) {
  byte cmd_params[] = {0xDA, 0x04};
  cmd_and_read_33(cmd_params, 2, rsp, 40);
}

void charger_cmd(byte rsp[]) {
  byte cmd_params[] = {0xF0, 0x00};
  cmd_and_read_cc(cmd_params, 2, rsp, 32);
}

void read_msg_cmd(byte rsp[]) {
  byte cmd_params[] = {0xAA, 0x00};
  cmd_and_read_33(cmd_params, 2, rsp, 40);
}

void clear_cmd(byte rsp[]) {
  byte cmd_params[] = {0xF0, 0x00};
  cmd_and_read_cc(cmd_params, 2, rsp, 0);
}

void store_cmd(byte rsp[]) {
  byte cmd_params[] = {0x55, 0xA5};
  cmd_and_read_33(cmd_params, 2, rsp, 0);
}

void clean_frame_cmd(byte rsp[]) {
  byte cmd_params[] = {0x33, 0x0F, 0x00, 0xF1, 0x26, 0xBD, 0x13, 0x14, 0x58,
                       0x00, 0x00, 0x94, 0x94, 0x40, 0x21, 0xD0, 0x80, 0x02,
                       0x4E, 0x23, 0xD0, 0x8E, 0x45, 0x60, 0x1A, 0x00, 0x03,
                       0x02, 0x02, 0x0E, 0x20, 0x00, 0x30, 0x01, 0x83};
  cmd_and_read_33(cmd_params, 34, rsp, 0);
}

void f0513_vcell1_cmd(byte rsp[]) {
  byte cmd_params[] = {0x31};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}
void f0513_vcell2_cmd(byte rsp[]) {
  byte cmd_params[] = {0x32};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}
void f0513_vcell3_cmd(byte rsp[]) {
  byte cmd_params[] = {0x33};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}
void f0513_vcell4_cmd(byte rsp[]) {
  byte cmd_params[] = {0x34};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}
void f0513_vcell5_cmd(byte rsp[]) {
  byte cmd_params[] = {0x35};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}
void f0513_temp_cmd(byte rsp[]) {
  byte cmd_params[] = {0x52};
  cmd_and_read_cc(cmd_params, 1, rsp, 2);
}

void f0513_model_cmd(byte rsp[]) {
  digitalWrite(ENABLEPIN, HIGH);
  delay(400);
  makita.reset();
  delayMicroseconds(400);
  makita.write(0xcc, 0);
  delayMicroseconds(90);
  makita.write(0x99, 0);
  delay(400);
  makita.reset();
  delayMicroseconds(400);
  makita.write(0x31, 0);
  delayMicroseconds(90);
  rsp[1] = makita.read();
  delayMicroseconds(90);
  rsp[0] = makita.read();
  delayMicroseconds(90);
  digitalWrite(ENABLEPIN, LOW);
  delay(400);
}

void f0513_version_cmd(byte rsp[]) {
  digitalWrite(ENABLEPIN, HIGH);
  delay(400);
  makita.reset();
  delayMicroseconds(400);
  makita.write(0xcc, 0);
  delayMicroseconds(90);
  makita.write(0x99, 0);
  delay(400);
  makita.reset();
  delayMicroseconds(400);
  makita.write(0x32, 0);
  delayMicroseconds(90);
  rsp[1] = makita.read();
  delayMicroseconds(90);
  rsp[0] = makita.read();
  delayMicroseconds(90);
  digitalWrite(ENABLEPIN, LOW);
  delay(400);
}

void f0513_testmode_cmd() {
  byte cmd_params[] = {0x99};
  byte rsp[8];
  memset(rsp, '\0', 8);

  cmd_and_read_cc(cmd_params, 1, rsp, 0);
}

bool f0513 = false;

void getModel() {
  byte data[64];
  char model[16];

  memset(data, '\0', 64);
  memset(model, '\0', 16);

  model_cmd(data);

  if (!(data[0] == 0xFF && data[1] == 0xFF)) {
    memcpy(model, data, 6);
  } else {
    f0513_model_cmd(data);
    sprintf(model, "BL%02x%02x", data[0], data[1]);
    f0513 = true;
  }

  DinMeter.Display.drawString("Model: " + String(model), 5, 5);
}

void getMsg() {
  byte data[64];
  char rom_id[32];
  byte raw_msg[32];
  char charge_count[32];
  char lock_flag[16];
  char status_code[8];
  char error_byte[8];
  int raw_count;

  memset(data, '\0', 64);
  memset(rom_id, '\0', 32);
  memset(raw_msg, '\0', 32);
  memset(charge_count, '\0', 32);
  memset(lock_flag, '\0', 16);
  memset(status_code, '\0', 8);
  memset(error_byte, '\0', 8);

  for(int i=0;i<5;i++){
     read_msg_cmd(data); 
     if(data[0]!=max(data[1],data[2]))break;
  }

  sprintf(rom_id, "%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2],
          data[3], data[4], data[5], data[6], data[7]);

  memcpy(raw_msg, data + 8, 32);

  raw_count = ((int)SWAP_NIBBLES((raw_msg[27]))) |
              ((int)SWAP_NIBBLES((raw_msg[26]))) << 8;

  sprintf(charge_count, "%0i", raw_count & 0x0fff);

  if (raw_msg[20] & 0x0f) {
    sprintf(lock_flag, "LOCKED");
  } else {
    sprintf(lock_flag, "UNLOCKED");
  }

  sprintf(error_byte, "%02x", raw_msg[19]);

  DinMeter.Display.drawString("ROM version: ", 5, 25);
  DinMeter.Display.drawString("   " + String(rom_id), 5, 45);
  DinMeter.Display.drawString("Charge count: " + String(charge_count), 5, 65);
  DinMeter.Display.drawString("State: " + String(lock_flag), 5, 85);
  DinMeter.Display.drawString("Status byte: " + String(error_byte), 5, 100);
}

void readSensors() {
  byte data[32];
  char str[256];
  memset(data, 0, 32);
  memset(str, 0, 256);

  if (f0513) {
    f0513_vcell1_cmd(data + 2);
    f0513_vcell2_cmd(data + 4);
    f0513_vcell3_cmd(data + 6);
    f0513_vcell4_cmd(data + 8);
    f0513_vcell5_cmd(data + 10);
    f0513_temp_cmd(data + 14);
  } else {
    read_data_request(data);
  }

  float v_pack = ((int)data[0] | ((int)data[1]) << 8) / 1000.0f;
  float v_cell1 = ((int)data[2] | ((int)data[3]) << 8) / 1000.0f;
  float v_cell2 = ((int)data[4] | ((int)data[5]) << 8) / 1000.0f;
  float v_cell3 = ((int)data[6] | ((int)data[7]) << 8) / 1000.0f;
  float v_cell4 = ((int)data[8] | ((int)data[9]) << 8) / 1000.0f;
  float v_cell5 = ((int)data[10] | ((int)data[11]) << 8) / 1000.0f;
  float v_diff = ((int)data[12] | ((int)data[13]) << 8) / 100000.0f;
  float t_pack = ((int)data[14] | ((int)data[15]) << 8) / 100.0f;
  float t_mosfet = ((int)data[16] | ((int)data[17]) << 8) / 100.0f;

  if (f0513) {
    v_pack = v_cell1 + v_cell2 + v_cell3 + v_cell4 + v_cell5;
    float max_v = max(max(max(v_cell1, v_cell2), max(v_cell2, v_cell3)),
                      max(v_cell4, v_cell5));
    float min_v = min(min(min(v_cell1, v_cell2), min(v_cell2, v_cell3)),
                      min(v_cell4, v_cell5));
    v_diff = max_v - min_v;
  }

  char s_v_pack[8] = {0};
  char s_v_cell1[8] = {0};
  char s_v_cell2[8] = {0};
  char s_v_cell3[8] = {0};
  char s_v_cell4[8] = {0};
  char s_v_cell5[8] = {0};
  char s_v_diff[8] = {0};
  char s_t_pack[8] = {0};
  char s_t_mosfet[8] = {0};

  dtostrf(v_pack, 4, 2, s_v_pack);
  dtostrf(v_cell1, 3, 2, s_v_cell1);
  dtostrf(v_cell2, 3, 2, s_v_cell2);
  dtostrf(v_cell3, 3, 2, s_v_cell3);
  dtostrf(v_cell4, 3, 2, s_v_cell4);
  dtostrf(v_cell5, 3, 2, s_v_cell5);
  dtostrf(v_diff, 4, 3, s_v_diff);
  dtostrf(t_pack, 4, 2, s_t_pack);
  dtostrf(t_mosfet, 4, 2, s_t_mosfet);

  DinMeter.Display.drawString("vPack: " + String(s_v_pack), 5, 3);
  DinMeter.Display.drawString("vDiff: " + String(s_v_diff), 5, 18);
  DinMeter.Display.drawString("tPack: " + String(s_t_pack), 5, 33);
  DinMeter.Display.drawString("tMosfet: " + String(s_t_mosfet), 5, 48);
  DinMeter.Display.drawString(
      "v1: " + String(s_v_cell1) + " v2: " + String(s_v_cell2), 5, 63);
  DinMeter.Display.drawString(
      "v3: " + String(s_v_cell3) + " v4: " + String(s_v_cell4), 5, 78);
  DinMeter.Display.drawString("v5: " + String(s_v_cell5), 5, 93);
}

int screen = 0;
long old_position = -999;
int autoTurnoff = 0;
void setup() {
  // One-wire
  pinMode(ENABLEPIN, OUTPUT);

  auto cfg = M5.config();
  DinMeter.begin(cfg, true);
  DinMeter.Display.setRotation(1);
  DinMeter.Display.setTextColor(GREEN);
  DinMeter.Display.setTextDatum(top_left);
  DinMeter.Display.setTextFont(&fonts::FreeMono9pt7b);

  DinMeter.Display.setTextSize(1);
  DinMeter.update();

  pinMode(1, INPUT);
  pinMode(2, OUTPUT);

  digitalWrite(1, LOW);
  digitalWrite(2, LOW);

  autoTurnoff=millis() + (TURNOFF_SECONDS*1000);
}



void loop() {
  DinMeter.update();

  long newPosition = DinMeter.Encoder.read();
  bool btnPressed = DinMeter.BtnA.wasPressed();
  bool newScr = false;

  if (DinMeter.BtnA.pressedFor(5000) ||(millis() > autoTurnoff)) {
    DinMeter.Power.powerOff();
    return;
  }

  if (abs(newPosition - old_position) > 2) {
    if (newPosition > old_position) {
      screen--;
    }

    if (newPosition < old_position) {
      screen++;
    }

    screen = screen < 0 ? 0 : screen;
    screen = screen > 4 ? 4 : screen;

    newScr = true;

    old_position = newPosition;
  }

  if (newScr || btnPressed) {
    autoTurnoff = millis() + (TURNOFF_SECONDS*1000);
  }

  switch (screen) {
    case 0:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        getModel();
        getMsg();
      }
      break;

    case 1:
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        readSensors();
      }
      break;

    case 2:
      if (newScr) {
        DinMeter.Display.clear();
        DinMeter.Display.drawString("Light up all leds.", 5, 5);
      }

      if (btnPressed) {
        if (f0513) {
          f0513_testmode_cmd();
        } else {
          testmode_cmd();
        }
        leds_on_cmd();
      }

      break;

    case 3:
      if (newScr) {
        DinMeter.Display.clear();
        DinMeter.Display.drawString("Turn off up all leds.", 5, 5);
      }
      if (btnPressed) {
        if (f0513) {
          f0513_testmode_cmd();
        } else {
          testmode_cmd();
        }
        leds_off_cmd();
      }
      break;

    case 4:
      if (newScr) {
        DinMeter.Display.clear();
        DinMeter.Display.drawString("Reset lockout.", 5, 5);
      }
      if (btnPressed) {
        if (f0513) {
          f0513_testmode_cmd();
        } else {
          testmode_cmd();
        }

        reset_error_cmd();
      }
      break;
  }
}