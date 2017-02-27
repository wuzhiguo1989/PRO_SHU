#ifndef __BEEP_H
#define __BEEP_H

#include "stc15wxxxx.h"
#include "utils.h"

#define SPEAK_TURN   P11
#define MUTE    P10
#define T4kHz	(MAIN_Fosc / 12 / 4000)
#define T1000Hz  (MAIN_Fosc / 12 / 2000)

#define mute_beep_enable()  MUTE = 1
#define mute_beep_disable() MUTE = 0 
#define speak_beep_disable() SPEAK_TURN = 0


void speak_beep_enable();
void speak_init_params();
void init_beep_io();
void detect_buzzer_io();
#endif