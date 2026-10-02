#ifndef EMOTES_H
#define EMOTES_H

#include <Arduino.h>

// 128x64 matrices (1024 bytes each). Initialized to generic patterns to satisfy C++ compilation.
const unsigned char epd_bitmap_booting[1024] PROGMEM = {0xAA}; // Alternating pixel pattern
const unsigned char epd_bitmap_happy[1024] PROGMEM = {0x00};   // Blank / Clear
const unsigned char epd_bitmap_angry[1024] PROGMEM = {0xFF};   // Solid block
const unsigned char epd_bitmap_offline[1024] PROGMEM = {0x55}; // Alternating inverted

#endif
