/*
 * Drop of Life
 * Julien Vanier <jvanier@gmail.com>
 */

#include "Particle.h"
#include "HT16K33-LED.h"
#include <time.h>
#include <stdlib.h>

PRODUCT_ID(3599);
PRODUCT_VERSION(1);

SYSTEM_THREAD(ENABLED);


HT16K33 display;
#define ROWS 16


void setup() {
  Serial.begin();
  setupStorage();
  setupRedCross();
  display.begin();
  display.setBrightness(9);
}

/* Persistent storage in the EEPROM */

const uint16_t DROP_OF_LIFE_APP = ('D'<<8 | 'L');
struct Storage {
  uint16_t app;
  uint8_t username[32];
  uint8_t password[32];
  uint8_t level;
} storage;

void setupStorage() {
  loadStorage();
  Particle.function("login", setCredentials);
}

void loadStorage() {
  EEPROM.get(0, storage);
  if (storage.app != DROP_OF_LIFE_APP) {
    storage.username[0] = '\0';
    storage.password[0] = '\0';
    storage.level = 0;
    storeStorage();
  }
}

void storeStorage() {
  EEPROM.put(0, storage);
}

int setCredentials(String arg) {
  int comma = arg.indexOf(",");
  if (comma < 0) {
    return -1;
  }
  String username = arg.substring(0, comma);
  String password = arg.substring(comma + 1);
  username.getBytes(storage.username, sizeof(storage.username));
  password.getBytes(storage.password, sizeof(storage.password));
  return 0;
}

/* Red Cros API interactions */

String token;

#define EVENT_RC_LOGIN "red_cross/login"
#define EVENT_RC_ELIGIBILITY "red_cross/eligibility"

time_t eligibility = 0;

void setupRedCross() {
  Particle.subscribe(
    System.deviceID() + "/hook-response/" EVENT_RC_LOGIN,
    setRedCrossToken
  );
  Particle.subscribe(
    System.deviceID() + "/hook-response/" EVENT_RC_ELIGIBILITY,
    setEligibility
  );
  loginToRedCross();
}

void setRedCrossToken(const char *event, const char *data) {
  token = data;
}

void loginToRedCross() {
  if (storage.username[0] == '\0' || storage.password[0] == '\0') {
    return;
  }
  String data = String::format(
    "{\"username\":\"%s\",\"password\":\"%s\"}",
    storage.username,
    storage.password
  );
  Particle.publish(EVENT_RC_LOGIN, data);
}

void updateEligibility() {
  if (token.length() == 0) {
    return;
  }
  String data = String::format("{\"token\":\"%s\"}", token.c_str());
  Particle.publish(EVENT_RC_ELIGIBILITY, data);
}

void setEligibility(const char *event, const char *data) {
  // convert string into time
  String dateStr = data;
  int dash1 = dateStr.indexOf("-");
  int dash2 = dateStr.indexOf("-", dash1+1);
  if (dash1 < 0 || dash2 < 0) {
    return;
  }
  int year = dateStr.substring(0, dash1).toInt();
  int month = dateStr.substring(dash1 + 1, dash2).toInt();
  int day = dateStr.substring(dash2 + 1).toInt();

  tm date;
  memset(&date, 0, sizeof(tm));
  date.tm_year = year - 1900;
  date.tm_mon = month - 1;
  date.tm_mday = day;

  eligibility = mktime(&date);
}

void loop() {
  static int level = 0;
  displayDrop(level);
  level++;
  delay(500);
  if (level == ROWS) {
    level = 0;
    delay(5000);
  }
}


const uint8_t DROP_FULL[ROWS] = {
  0b00010000,
  0b00010000,
  0b00111000,
  0b00111000,
  0b01111100,
  0b01111100,
  0b01111110,
  0b11111110,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00111100,
};

const uint8_t DROP_EMPTY[ROWS] = {
  0b00010000,
  0b00010000,
  0b00101000,
  0b00101000,
  0b01000100,
  0b01000100,
  0b01000010,
  0b10000010,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b01000010,
  0b00111100,
};

const uint8_t LINE_TO_ROW[ROWS] = {
  15,
  13,
  11,
  9,
  7,
  5,
  3,
  1,
  14,
  12,
  10,
  8,
  6,
  4,
  2,
  0,
};

void displayDrop(uint8_t level) {
  uint8_t lines[ROWS];
  for (int i = 0; i < ROWS; i++) {
    uint8_t row = LINE_TO_ROW[i];
    lines[row] = (i < ROWS - level) ? DROP_EMPTY[i] : DROP_FULL[i];
  }
  display.writeDisplay(lines, 0, ROWS);
}
