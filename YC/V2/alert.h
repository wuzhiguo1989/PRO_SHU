#ifndef     __ALERT_H
#define     __ALERT_H

#include "stc15w4k32s4.h"
#include "config.h"

#define	ALERT_BEEP      P24
// #define ALERT_FEED_LACK P23
#define ALERT_FEED_LACK  INT1
#define OVER_LOAD_IO  P23
#define alert_beep_enable()   ALERT_BEEP = 1
#define alert_beep_disable()   ALERT_BEEP = 0


#define ALTER_FULL_FEED  0x02
#define ALTER_LACK_FEED  0x01
#define ALTER_OVER_LOAD  0x04
#define ALTER_TEM_BEEP   0x08
#define ALTER_TEM_STOP   0x10

extern u8 alert_for_system;
extern u8 record;

void get_RTC_from_screen();
void init_io_alert();
void alert_for_temperature();
void alert_for_overload();
void alert_beep();
#endif