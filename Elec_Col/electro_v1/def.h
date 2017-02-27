# ifndef __DEF_H_
# define __DEF_H_

#include "stc15wxxxx.h"


typedef struct time_entity
{
	u8 second;
	u8 minute;		/* minutes, range 0 to 59           */
	u8 hour;		/* hours, range 0 to 23             */
} time_t;

typedef enum speak_mode_type {
	mute = 0,
	unlockmode,
	lookformode,
	lockmode,
	firstalertmode,
	secondalertmode,
	normal,
	buzzer,
	light_alert
} speak_t;




# endif