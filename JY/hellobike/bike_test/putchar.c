#include "lm567.h"
#include "stdio.h"
#include "stc15wxxxx.h"
char putchar (char c)
{
	return TX1_write2buff(c);
}
