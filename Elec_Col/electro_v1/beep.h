#ifndef __BEEP_H
#define __BEEP_H

#include "stc15wxxxx.h"
#include "utils.h"

#define SPEAK   P26
#define SPEAK_TURN   P24
#define MUTE    P25
#define BUZZER  P15
#define T4kHz	(MAIN_Fosc / 12 / 4000)
#define T500Hz  (MAIN_Fosc / 12 / 1000)
#define T600Hz  (MAIN_Fosc / 12 / 1200)
#define T800Hz  (MAIN_Fosc / 12 / 1600)
#define T1000Hz  (MAIN_Fosc / 12 / 2000)

#define mute_beep_enable()  MUTE = 1
#define mute_beep_disable() MUTE = 0 
#define speak_beep_disable() SPEAK = 0; SPEAK_TURN = 0


void speak_beep_enable();
void speak_init_params();
void init_beep_io();
void detect_buzzer_io();
#endif