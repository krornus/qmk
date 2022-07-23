#ifndef ripple_h_INCLUDED
#define ripple_h_INCLUDED

#ifndef QMK_EMULATOR
#include QMK_KEYBOARD_H
#else
#include "emu/qmk.h"
#endif

void process_record_ripples(keyrecord_t *record);
void oled_write_ripples(void);

#endif // ripple_h_INCLUDED

