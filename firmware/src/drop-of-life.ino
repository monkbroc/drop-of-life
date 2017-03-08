/*
 * Drop of Life
 * Julien Vanier <jvanier@gmail.com>
 */

#include "Particle.h"
SYSTEM_THREAD(ENABLED);
#include "HT16K33-LED.h"

HT16K33 display;

void setup() {
  display.begin();
  display.setBrightness(9);
}

void loop() {
displayDrop(16);
}

#define ROWS 16

const uint8_t DROP_FULL[ROWS] = {
  0b00010000,
  0b00010000,
  0b00111000,
  0b00111000,
  0b01111100,
  0b01111100,
  0b01111110,
  0b01111110,
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
  0b01000010,
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
    lines[row] = DROP_FULL[i];
  }
  display.writeDisplay(lines, 0, ROWS);
}
