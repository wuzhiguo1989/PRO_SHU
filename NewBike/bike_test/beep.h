#ifndef __BEEP_H
#define __BEEP_H

#include "stc15wxxxx.h"
#include "utils.h"

#define SPEAK   P36
#define T100Hz  (MAIN_Fosc / 12 / 100)
#define T800Hz  (MAIN_Fosc / 12 / 2000)
#define T600Hz  (MAIN_Fosc / 12 / 2000)
#define T4kHz  (MAIN_Fosc / 12 / 4000)
#define T2kHz  (MAIN_Fosc / 12 / 2000)

typedef enum speak_mode_type {
	mute = 0,
	alter_beep,
	trace_beep,
	schedule_beep,
	open_beep,
	lock_beep
} speak_t;

typedef enum lock_status {
	off = 0,
	on = 1
} lock_st;


extern volatile speak_t speak_mode;
extern volatile lock_st status;

#define speak_beep_disable()  SPEAK = 0
#define speak_beep_enable() SPEAK = ~SPEAK

void speak_init_params();
void init_beep_io();
void speak_alter_pca_init();

#endif