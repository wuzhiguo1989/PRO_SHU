#ifndef __KEY_H_
#define __KEY_H_

#include "stc15wxxxx.h"

#define MB    P32
#define CHG   P15
#define STY   P14
#define POUT  P54
#define MUTE  P16
#define SPEAK P17

#define MB_ID 0x01

#define LONG_KEY_TIME   100
#define SHORT_KEY_TIME  25

#define T4kHz	(MAIN_Fosc / 12 / 4000)
#define T1000Hz  (MAIN_Fosc / 12 / 1200)

#define mute_beep_enable()  MUTE = 1
#define mute_beep_disable() MUTE = 0 
#define speak_beep_disable() SPEAK = 0
#define speak_beep_enable() SPEAK = ~SPEAK

void speak_init_params();
void key_io_init();
void key_scan_task();
void key_duty_task();

#endif