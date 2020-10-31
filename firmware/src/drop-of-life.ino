/*
 * Drop of Life
 * Julien Vanier <jvanier@gmail.com>
 */

#include "Particle.h"
#include "HT16K33-LED.h"
#include <time.h>
#include <stdlib.h>

PRODUCT_ID(3599);
PRODUCT_VERSION(3);

SYSTEM_THREAD(ENABLED);

constexpr long minutesToMs(long minutes) { return minutes * 60 * 1000; }
constexpr long weekToSeconds(long week) { return week * 7 * 24 * 60 * 60; }

HT16K33 display;
const int ROWS = 16;
const int MAX_LEVEL = ROWS - 1;
const long ELIGIBILITY_UPDATE_INTERVAL = minutesToMs(60);
const long REDCROSS_DONATION_INTERVAL = weekToSeconds(8);

int level = 0;

void setup() {
  Serial.begin();
  setupStorage();
  setupDisplay();
}

void loop() {
  processCloud();
  processRedCross();
  processDisplay();
}

void processCloud() {
  static bool didConnect = false;
  if (!didConnect && Particle.connected()) {
    registerStorage();
    registerRedCross();
    registerDisplay();
    didConnect = true;
  }
}

/* Persistent storage in the EEPROM */

const uint16_t DROP_OF_LIFE_APP = ('D'<<8 | 'L');
struct Storage {
  uint16_t app;
  uint8_t username[32];
  uint8_t password[32];
} storage;

void setupStorage() {
  loadStorage();
}

void registerStorage() {
  Particle.function("login", setCredentials);
}

void loadStorage() {
  EEPROM.get(0, storage);
  if (storage.app != DROP_OF_LIFE_APP) {
    storage.app = DROP_OF_LIFE_APP;
    storage.username[0] = '\0';
    storage.password[0] = '\0';
    storeStorage();
  }
}

void storeStorage() {
  EEPROM.put(0, storage);
}

int setCredentials(String arg) {
  Serial.println("Set credentials");
  int comma = arg.indexOf(",");
  if (comma < 0) {
    return -1;
  }
  String username = arg.substring(0, comma);
  String password = arg.substring(comma + 1);
  username.getBytes(storage.username, sizeof(storage.username));
  password.getBytes(storage.password, sizeof(storage.password));
  storeStorage();
  return 0;
}

/* Red Cros API interactions */

String token;

#define EVENT_RC_LOGIN "red_cross/login"
#define EVENT_RC_ELIGIBILITY "red_cross/eligibility"

time_t eligibility = 0;

void registerRedCross() {
  Particle.subscribe(
    System.deviceID() + "/hook-response/" EVENT_RC_LOGIN,
    setRedCrossToken,
    MY_DEVICES
  );
  Particle.subscribe(
    System.deviceID() + "/hook-response/" EVENT_RC_ELIGIBILITY,
    setEligibility,
    MY_DEVICES
  );
}

void processRedCross() {
  static bool didLogin = false;
  static long lastUpdate = -ELIGIBILITY_UPDATE_INTERVAL;

  if (!Particle.connected()) {
    return;
  }

  if (!didLogin) {
    if (loginToRedCross()) {
      didLogin = true;
    }
  }

  if (didLogin && (millis() - lastUpdate > ELIGIBILITY_UPDATE_INTERVAL)) {
    if (updateEligibility()) {
      lastUpdate = millis();
    }
  }

  setLevelFromEligibility();
}

void setLevelFromEligibility() {
  long now = Time.now();
  if (eligibility != 0 && now != 0) {
    if (eligibility < now) {
      level = MAX_LEVEL;
    } else {
      level = MAX_LEVEL - ((eligibility - now) * MAX_LEVEL / REDCROSS_DONATION_INTERVAL);
    }
  }
}

void setRedCrossToken(const char *event, const char *data) {
  Serial.println("Got token");
  token = data;
}

bool loginToRedCross() {
  if (storage.username[0] == '\0' || storage.password[0] == '\0') {
    return false;
  }
  Serial.println("Publishing RC login event");
  String data = String::format(
    "{\"username\":\"%s\",\"password\":\"%s\"}",
    storage.username,
    storage.password
  );
  Particle.publish(EVENT_RC_LOGIN, data, PRIVATE);
  return true;
}

bool updateEligibility() {
  if (token.length() == 0) {
    return false;
  }
  Serial.println("Publishing RC get token event");
  String data = String::format("{\"token\":\"%s\"}", token.c_str());
  Particle.publish(EVENT_RC_ELIGIBILITY, data, PRIVATE);
  return true;
}

void setEligibility(const char *event, const char *data) {
  Serial.println("Got eligibility date " + String(data));
  // convert string into time
  String dateStr = data;
  int dash1 = dateStr.indexOf("/");
  int dash2 = dateStr.indexOf("/", dash1+1);
  if (dash1 < 0 || dash2 < 0) {
    return;
  }
  int month = dateStr.substring(0, dash1).toInt();
  int day = dateStr.substring(dash1 + 1, dash2).toInt();
  int year = dateStr.substring(dash2 + 1).toInt();

  tm date;
  memset(&date, 0, sizeof(tm));
  date.tm_year = year - 1900;
  date.tm_mon = month - 1;
  date.tm_mday = day;

  eligibility = mktime(&date);
}

/* Display */

void setupDisplay() {
  display.begin();
}

void registerDisplay() {
  Particle.function("demo", startDemo);
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

/* run demo once at startup */
int demoLevel = 0;
long demoTime = 0;

int startDemo(String) {
  demoLevel = 0;
  demoTime = millis();
  return 0;
}

void processDisplay() {
  if (demoLevel >= 0) {
    runDemo();
  } else {
    displayDrop(level);
  }
}

void runDemo() {
  static const long DEMO_DELAY = 1000;
  displayDrop(demoLevel);
  if (millis() - demoTime > DEMO_DELAY) {
    demoLevel++;
    demoTime = millis();
  }

  if (demoLevel >= MAX_LEVEL + 15) {
    demoLevel = -1;
  }
}

void displayDrop(uint8_t level) {
  uint8_t lines[ROWS];
  for (int i = 0; i < ROWS; i++) {
    uint8_t row = LINE_TO_ROW[i];
    lines[row] = (i < ROWS - level) ? DROP_EMPTY[i] : DROP_FULL[i];
  }
  display.writeDisplay(lines, 0, ROWS);
  updateBrightness(level);
}

void updateBrightness(uint8_t level) {
  static const int defaultBrightness = 9;
  static int brightness = 9;
  static const long FADE_INTERVAL = 300;
  static int fadeDirection = 1;
  static const int maxFadeBrightness = 14;
  static const int minFadeBrightness = 6;
  static long fadeTime = 0;

  if (level < MAX_LEVEL) {
    brightness = defaultBrightness;
  } else {
    if (millis() - fadeTime > FADE_INTERVAL) {
      brightness += fadeDirection;
      if (brightness >= maxFadeBrightness) {
        fadeDirection = -1;
      } else if (brightness <= minFadeBrightness) {
        fadeDirection = 1;
      }
      fadeTime = millis();
    }
  }
  display.setBrightness(brightness);
}
