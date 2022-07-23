#ifndef oled_h_INCLUDED
#define oled_h_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define PROGMEM

typedef struct {
    uint8_t col;
    uint8_t row;
} keypos_t;

typedef struct {
    keypos_t key;
    bool     pressed;
    uint16_t time;
} keyevent_t;

typedef struct {
    bool    interrupted : 1;
    bool    reserved2 : 1;
    bool    reserved1 : 1;
    bool    reserved0 : 1;
    uint8_t count : 4;
} tap_t;

typedef struct {
    keyevent_t event;
    tap_t tap;
    uint16_t keycode;
} keyrecord_t;

void init(void);
void destroy(void);

void oled_write_pixel(uint8_t x, uint8_t y, bool on);

void oled_clear(void);

uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t last);

#endif // oled_h_INCLUDED