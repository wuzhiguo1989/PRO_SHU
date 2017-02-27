# ifndef __DEF_H_
# define __DEF_H_

#include "stc15wxxxx.h"


typedef struct time_entity
{
	u8 minute;		/* minutes, range 0 to 59           */
	u8 second;		/* hours, range 0 to 23             */
} time_t;

# endif