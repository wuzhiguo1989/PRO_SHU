#ifndef __UTILS_H_
#define __UTILS_H_

#include "stc15wxxxx.h"

#define high_at_bit(r, bit)		r |= bit
#define low_at_bit(r, bit)		r &= ~bit

#define set_quasi_bidirectional_on(x, pin) 		low_at_bit(P##x##M1, pin); low_at_bit(P##x##M0, pin)
#define set_push_pull_output_on(x, pin)			low_at_bit(P##x##M1, pin); high_at_bit(P##x##M0, pin)
#define set_input_only_on(x, pin)				high_at_bit(P##x##M1, pin); low_at_bit(P##x##M0, pin)
#define set_open_drain_on(x, pin)				high_at_bit(P##x##M1, pin); high_at_bit(P##x##M0, pin)

#define pure_input(x, pin)						set_input_only_on(x, pin)
#define push_pull(x, pin)						set_push_pull_output_on(x, pin)
#define quasi_bidirectional(x, pin)				set_quasi_bidirectional_on(x, pin)
#define open_drain(x, pin)						set_open_drain_on(x, pin)

#define LOW                     0
#define HIGH                    1
#define EVEN                    0
#define ODD                     1
#define ON 						1
#define OFF 					0
#define SUCCESS     			1
#define FAIL        			0
#define NO 						0
#define YES 					1

#define switch_on(pin) low(pin)
#define switch_off(pin) high(pin)

// flag set to 0
// clear set to 1
#define  WDT_FLAG	0x80
#define  CLR_WDT	0x10

#define reset_watch_dog()     WDT_CONTR &= ~WDT_FLAG;	WDT_CONTR |= CLR_WDT

#endif