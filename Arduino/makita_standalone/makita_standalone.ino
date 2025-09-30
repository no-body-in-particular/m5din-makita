#include <EEPROM.h>
#include "M5DinMeter.h"
#include <OneWire2.h>


// pins used by this application, change as needed
#define MAKITAPIN 1
#define ENABLEPIN 2

#define SWAP_NIBBLES(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
#define TURNOFF_SECONDS 60  // seconds without input before the screen turns off

// global variables
OneWire<1> makita =OneWire<1>();

void set_enablepin(bool high) {
  uint8_t val = high ? HIGH : LOW;
  digitalWrite(ENABLEPIN, val);

  if(!high){
    digitalWrite(MAKITAPIN, LOW);
  }

  delay(400);
}

void  cmd_and_read_33(byte *cmd, uint8_t cmd_len, byte *rsp,
                                 uint8_t rsp_len) {
  makita.reset();
  delayMicroseconds(310);
  makita.write(0x33);
  makita.read_bytes(rsp, 8);
  makita.write_bytes(cmd, cmd_len);
  makita.read_bytes(rsp + 8, rsp_len);
  delay(100);
}

void  cmd_and_read_cc(byte *cmd, uint8_t cmd_len, byte *rsp,
                                 uint8_t rsp_len) {
  makita.reset();
  delayMicroseconds(310);
  makita.skip();
  makita.write_bytes(cmd, cmd_len);
  makita.read_bytes(rsp, rsp_len);
  delay(100);
}

void  f0513_second_command_tree() {
  byte cmd[] = {0x99};
  byte rsp[16];
  memset(rsp, 0, 16);
  cmd_and_read_cc(cmd, 1, rsp, 0);
  makita.reset();
  delayMicroseconds(310);
}

void  f0513_model_cmd(byte rsp[]) {
  f0513_second_command_tree();
  makita.write(0x31);
  makita.read_bytes(rsp, 2);
}

void  f0513_version_cmd(byte rsp[]) {
  f0513_second_command_tree();
  makita.write(0x32);
  makita.read_bytes(rsp, 2);
}

// request to read model information ( BL18xx name)
void model_cmd(byte rsp[]) {
  byte cmd_params[] = {0xDC, 0x0C};
  cmd_and_read_cc(cmd_params, 2, rsp, 10);
}

// request to read voltages/sensor data
void read_data_request(byte rsp[]) {
  byte cmd_params[] = {0xD7, 0x00, 0x00, 0xFF};
  cmd_and_read_cc(cmd_params, 4, rsp, 29);
}

// first 8 bytes are ID, rest is MSG
void charger_33_cmd(byte rsp[]) {
  byte cmd_params[] = {0xF0, 0x00};
  cmd_and_read_33(cmd_params, 2, rsp, 32);
}

bool try_charger(byte rsp[]) {
  for (int i = 0; i < 5; i++) {
    memset(rsp, 0xff, 40);
    charger_33_cmd(rsp);
    if (!(rsp[8] == 0xFF && rsp[9] == 0xFF && rsp[10]==0xff)) return true;
    set_enablepin(false);
    set_enablepin(true);
  }

  return false;
}

// request to enable test mode
void testmode_cmd() {
  byte cmd_params[] = {0xD9, 0x96, 0xA5};
  byte rsp[64];
  memset(rsp, '\0', 64);

  cmd_and_read_33(cmd_params, 3, rsp, 29);
}

void reset_error_cmd() {
  byte cmd_params[] = {0xDA, 0x04};
  byte rsp[32];
  memset(rsp, '\0', 32);
  cmd_and_read_33(cmd_params, 2, rsp, 9);
}

void charger_cmd(byte rsp[]) {
  byte cmd_params[] = {0xF0, 0x00};
  cmd_and_read_cc(cmd_params, 2, rsp, 32);
}

// Read the device ID and message block. Same as the charger command, except
// more device specific.
void read_msg_cmd(byte rsp[]) {
  byte cmd_params[] = {0xAA, 0x00};
  cmd_and_read_33(cmd_params, 2, rsp, 40);
}

void store_cmd(byte data[]) {
  byte cmd_params[64];
  byte rsp[16];

  cmd_params[0] = 0x0F;
  cmd_params[1] = 0x00;
  memcpy(cmd_params + 2, data, 32);
  cmd_and_read_33(cmd_params, 34, rsp, 0);

  cmd_params[0] = 0x55;
  cmd_params[1] = 0xA5;
  cmd_and_read_cc(cmd_params, 2, rsp, 0);
  // 0x55, 0xA5
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

void f0513_testmode_cmd() {
  byte cmd_params[] = {0x99};
  byte rsp[8];
  memset(rsp, '\0', 8);

  cmd_and_read_cc(cmd_params, 1, rsp, 0);
}

bool is_f0513() {
  byte data[32];
  f0513_model_cmd(data);
  set_enablepin(false);
  set_enablepin(true);
  return !(data[0] == 0xFF && data[1] == 0xFF);
}

int round5(int in) {
  static signed char cAdd[5] = {0, -1, -2, 2, 1};
  return in + cAdd[in % 5];
}

void getModel() {
  byte data[64];
  char model[16];
  bool got_model = false;

  memset(data, '\0', 64);
  memset(model, '\0', 16);

  model_cmd(data);

  if (!(data[0] == 0xFF && data[1] == 0xFF)) {
    memcpy(model, data, 6);
    got_model = true;
  }

  if (!got_model) {
    set_enablepin(false);
    set_enablepin(true);
    f0513_model_cmd(data);
  }

  if (!got_model && !(data[0] == 0xFF && data[1] == 0xFF)) {
    sprintf(model, "BL%02x%02x", data[1], data[0]);
    got_model = true;
  }

  if (!got_model) {
    set_enablepin(false);
    set_enablepin(true);

    // unknown type, extract model from msg. usually this is a 14v model -
    // sometimes it's a chinese clone of a 18v battery ( number 0D in byte 17 )
    if (!try_charger(data)) {
      DinMeter.Display.drawString("Unsupported battery.", 5, 3);
      return;  // if the charger command doesn't work we're borked.
    }

    int cap = round5(SWAP_NIBBLES(data[24]));

    if (SWAP_NIBBLES(data[25]) < 0xC) {
      sprintf(model, "BL14%02d", cap);
    } else {
      sprintf(model, "BL18%02d", cap);
    }
  }

  DinMeter.Display.drawString("Model: " + String(model), 5, 3);
}

bool has_health() {
  byte rsp[4];
  memset(rsp, 0, 4);
  byte cmd_params[] = {0xD4, 0xBA, 0x00, 0x01};
  set_enablepin(false);
  set_enablepin(true);
  cmd_and_read_cc(cmd_params, 4, rsp, 2);

  return rsp[1] == 0x06;
}

byte overload() {
  byte rsp[16];
  memset(rsp, 0, 4);
  byte cmd_params[] = {0xD4, 0x8D, 0x00, 0x07};
  set_enablepin(false);
  set_enablepin(true);
  cmd_and_read_cc(cmd_params, 4, rsp, 8);
  return (rsp[5] & 0xf0) >> 4 | (rsp[6] & 0x70);
}

byte overdischarge() {
  byte rsp[4];
  memset(rsp, 0, 4);
  byte cmd_params[] = {0xD4, 0xBA, 0x00, 0x01};
  set_enablepin(false);
  set_enablepin(true);
  cmd_and_read_cc(cmd_params, 4, rsp, 2);

  return rsp[0] << 1;
}

byte health() {
  byte rsp[4];
  memset(rsp, 0, 4);
  byte cmd_params[] = {0xD4, 0x50, 0x01, 0x02};
  set_enablepin(false);
  set_enablepin(true);
  cmd_and_read_cc(cmd_params, 4, rsp, 3);

  return min(14 * (rsp[1] - 10), 100);
}

float cell_temperature() {
  byte rsp[4];
  memset(rsp, 0, 4);
  byte cmd_params[] = {0xD7, 0x0E, 0x00, 0x02};
  set_enablepin(false);
  set_enablepin(true);
  cmd_and_read_cc(cmd_params, 4, rsp, 3);
  return ( ((rsp[0]) | ((int32_t)rsp[1])<<8)/10  ) -273.15f  ;
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

  if (!try_charger(data)) return;

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

  sprintf(error_byte, "%02x", raw_msg[19] & 0x0f);

  // design capacity in MaH
  uint32_t design_capacity = SWAP_NIBBLES(raw_msg[16]) * 100l;

  uint8_t overload_percent = raw_msg[25];
  overload_percent = SWAP_NIBBLES(overload_percent);
  overload_percent = overload_percent & 0xE0 ? overload_percent & 0x1F : 0;
  overload_percent *= 5;

  uint8_t undervoltage_percent = raw_msg[24] & 1 ? (~raw_msg[24]) >> 4 : 0;
  undervoltage_percent *= 5.33f;

  uint8_t health_percent =
      100 -
      (raw_count / 8.96f);  // reduces 25% every 224 cycles for older batteries

  // newer battery with health data
  if (has_health()) {
    health_percent = health();
    undervoltage_percent = overdischarge();
    overload_percent = overload();
  }

  DinMeter.Display.drawString("Charge count: " + String(charge_count), 5, 18);
  DinMeter.Display.drawString("State: " + String(lock_flag), 5, 33);
  DinMeter.Display.drawString("Status byte: " + String(error_byte), 5, 48);
  DinMeter.Display.drawString("DesignCap: " + String(design_capacity) + "MaH",
                              5, 63);
  DinMeter.Display.drawString("Overload: " + String(overload_percent) + "%", 5,
                              78);
  DinMeter.Display.drawString(
      "Overdischarge: " + String(undervoltage_percent) + "%", 5, 93);
  DinMeter.Display.drawString("Health: " + String(health_percent) + "%", 5,
                              108);
}

bool get_voltage_info(float output[]) {
  bool f0513 = false;
  uint8_t data[128];
  memset(data, 0, 128);

  if (!f0513) {
    read_data_request(data);
  }

  float t_pack  =0;

  if (data[0] == 0xff && data[1] == 0xff) {
    memset(data, 0xff, 32);
    f0513 = true;
    set_enablepin(false);
    delay(400);
    set_enablepin(true);
  }else{
    t_pack = cell_temperature();
  }

  if (f0513) {
    f0513_vcell1_cmd(data + 2);
    f0513_vcell2_cmd(data + 4);
    f0513_vcell3_cmd(data + 6);
    f0513_vcell4_cmd(data + 8);
    f0513_vcell5_cmd(data + 10);
    f0513_temp_cmd(data + 14);
    t_pack= ((int)data[14] | ((int)data[15]) << 8) / 100.0f;
  }

  if (data[2] == 0xff && data[3] == 0xff) return false;

  float v_pack = ((int)data[0] | ((int)data[1]) << 8) / 1000.0f;
  float v_cell1 = ((int)data[2] | ((int)data[3]) << 8) / 1000.0f;
  float v_cell2 = ((int)data[4] | ((int)data[5]) << 8) / 1000.0f;
  float v_cell3 = ((int)data[6] | ((int)data[7]) << 8) / 1000.0f;
  float v_cell4 = ((int)data[8] | ((int)data[9]) << 8) / 1000.0f;
  float v_cell5 = ((int)data[10] | ((int)data[11]) << 8) / 1000.0f;
  float v_diff = 0;

  if (f0513) {
    v_pack = v_cell1 + v_cell2 + v_cell3 + v_cell4 + v_cell5;
  }

  float max_v = max(max(max(v_cell1, v_cell2), max(v_cell2, v_cell3)),
                      max(v_cell4, v_cell5));
   float min_v = min(min(min(v_cell1, v_cell2), min(v_cell2, v_cell3)),
                      min(v_cell4, v_cell5));
  v_diff = max_v - min_v;


  output[0] = v_pack;
  output[1] = v_cell1;
  output[2] = v_cell2;
  output[3] = v_cell3;
  output[4] = v_cell4;
  output[5] = v_cell5;
  output[6] = v_diff;
  output[7] = t_pack;
  return true;
}

void readSensors() {
  float data[8];
  memset(data, 0, sizeof(data));

  if (!get_voltage_info(data)) {
    DinMeter.Display.drawString("Sensors not supported", 5, 3);
    return;  // if the charger command doesn't work we're borked.
  }
  DinMeter.Display.drawString("vPack: " + String(data[0]), 5, 3);
  DinMeter.Display.drawString("vDiff: " + String(data[6]), 5, 18);
  DinMeter.Display.drawString("tPack: " + String(data[7]), 5, 33);
  DinMeter.Display.drawString(
      "v1: " + String(data[1]) + " v2: " + String(data[2]), 5, 48);
  DinMeter.Display.drawString(
      "v3: " + String(data[3]) + " v4: " + String(data[4]), 5, 63);
  DinMeter.Display.drawString("v5: " + String(data[5]), 5, 78);
}

void diagnosis() {
  float voltage_data[8];
  memset(voltage_data, 0, sizeof(voltage_data));
  int cause = 0;
  float maxv = -100;
  float minv = 100;
  bool error_byte_set = false;
  byte data[64];
  
  memset(data, 0, 64);

  if (try_charger(data)) {
    uint8_t error_byte = (data[28] & 0x0f) ;
    if (error_byte) {
      error_byte_set = true;
    }
  }

  if (get_voltage_info(voltage_data)) {
    for (int i = 1; i < 6; i++) {
      if (voltage_data[i] < 3.0f) {
        cause = 1;
      }
      maxv = max(maxv, voltage_data[i]);
      minv = min(minv, voltage_data[i]);
    }

    if ( error_byte_set && (voltage_data[6] > 0.15f)) {
      // large cell voltage difference
      cause = 2;

      // board has broken balance circuit
      if (abs(maxv - minv) < 0.1) {
        cause = 3;
      }
    }

    // overtemperature
    if (voltage_data[7] > 40.0f) {
      cause = 4;
    }
  }

  if (error_byte_set) {
     if (cause == 0) cause = 5;
  }

  int model_type = 0;

  if (has_health()) {
    model_type = 1;
  }

  if (is_f0513()) {
    model_type = 2;
  }


  DinMeter.Display.drawString("Failure analysis: ", 5, 5);

  if (cause > 0 && error_byte_set && model_type == 2) {
    DinMeter.Display.drawString(" F0513. Error reset", 5, 48);
    DinMeter.Display.drawString("   unsupported.", 5, 63);
    return;
  }

  if (cause > 0 && error_byte_set && model_type == 0) {
    DinMeter.Display.drawString(" Old model & ", 5, 18);
    DinMeter.Display.drawString("   Lockout set", 5, 33);
    DinMeter.Display.drawString(" Reset could work.", 5, 48);
    DinMeter.Display.drawString(" PCB likely has ",5,63);
    DinMeter.Display.drawString( "  no undervoltage", 5, 78);
    DinMeter.Display.drawString( "  protection.", 5, 93);
    DinMeter.Display.drawString(" replace pcb.", 5, 108);
    return;
  }

  switch (cause) {
    case 0:
      DinMeter.Display.drawString(" No failure.", 5, 18);
      break;
    case 1:
      DinMeter.Display.drawString(" Cell undervoltage.", 5, 18);
      if (model_type == 2)
        DinMeter.Display.drawString(" -Manually charge", 5, 33);
      DinMeter.Display.drawString(" -Reset battery.", 5, 48);
      break;
    case 2:
      DinMeter.Display.drawString(". Cell out of balance.", 5, 18);
      DinMeter.Display.drawString(" -Balance cells.", 5, 33);
      DinMeter.Display.drawString(" -Reset battery.", 5, 48);
      break;
    case 3:
      DinMeter.Display.drawString(" Balance circuit", 5, 18);
      DinMeter.Display.drawString("    broken", 5, 33);
      DinMeter.Display.drawString(" -Repair circuit.", 5, 48);
      DinMeter.Display.drawString(" -Reset battery.", 5, 63);
      break;
    case 4:
      DinMeter.Display.drawString(" Battery overheated", 5, 18);
      DinMeter.Display.drawString(" -Let battery cool.", 5, 33);
      break;
    case 5:
      DinMeter.Display.drawString(" Chip Error", 5, 18);
      DinMeter.Display.drawString(" -Reset battery.", 5, 33);
      break;
  }
}

void setup() {
  auto cfg = M5.config();
  DinMeter.begin(cfg, true);
  DinMeter.Display.setRotation(1);
  DinMeter.Display.setTextColor(GREEN);
  DinMeter.Display.setTextDatum(top_left);
  DinMeter.Display.setTextFont(&fonts::FreeMono9pt7b);

  DinMeter.Display.setTextSize(1);
  DinMeter.update();

  pinMode(MAKITAPIN, INPUT);
  pinMode(ENABLEPIN, OUTPUT);

  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
}

void showDone() {
  DinMeter.Display.clear();
  DinMeter.Display.drawString("done.", 20, 20);
  delay(2000);
}

void loop() {
  // variables we update each loop
  static int screen = 0;
  static long old_position = DinMeter.Encoder.read();
  static int autoTurnoff = -1;
  static bool nextNewScr = true;

  if (autoTurnoff < 0) {
    autoTurnoff = millis() + (TURNOFF_SECONDS * 1000);
  }

  DinMeter.update();

  long newPosition = DinMeter.Encoder.read();
  bool btnPressed = DinMeter.BtnA.wasPressed();
  bool newScr = false;

  if (DinMeter.BtnA.pressedFor(5000) || (millis() > autoTurnoff)) {
    DinMeter.Power.powerOff();
    return;
  }

  if (abs(newPosition - old_position) > 2) {
    if (newPosition < old_position) {
      screen++;
    }

    if (newPosition > old_position) {
      screen--;
    }

    screen = screen < 0 ? 3 : screen;
    screen = screen > 3 ? 3 : screen;

    newScr = true;

    old_position = newPosition;
  }

  if (nextNewScr) {
    newScr = true;
    nextNewScr = false;
  }

  if (newScr || btnPressed) {
    autoTurnoff = millis() + (TURNOFF_SECONDS * 1000);
    set_enablepin(true);
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
      if (btnPressed || newScr) {
        DinMeter.Display.clear();
        diagnosis();
      }
      break;

    case 3:
      if (newScr) {
        DinMeter.Display.clear();
        DinMeter.Display.drawString("Reset lockout.", 5, 5);
      }
      if (btnPressed) {
        for(int i=0;i<3;i++){
           delay(300);
           testmode_cmd();
           reset_error_cmd();
           set_enablepin(true);
            set_enablepin(false);
        }
        showDone();
        nextNewScr = true;
      }
      break;
  }
  if (btnPressed || newScr) {
    set_enablepin(false);
  }
}
