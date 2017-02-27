#include "extension.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// static char *olds;
// char * STRTOK(char *s, const char *delim)
// {
// 	char *token;

// 	if (s == NULL)
// 		s = olds;

// 	s += strspn (s, delim);
// 	if (*s == '\0')
// 		{
// 			olds = s;
// 			return NULL;
// 		}

// 	token = s;
// 	s = strpbrk (token, delim);
// 	if (s == NULL)
// 		olds = memchr (token, '\0', 256);
// 	else
// 		{
// 			*s = '\0';
// 			olds = s + 1;
// 		}
// 	return token;
// }

// char xdata strtok_test[] = "AT+HTTPPARA=\"URL\",\"api.yeelink.net/v1.0/device/344230/sensor/382670/datapoints\"\r\n\r\nOK\r\n";
// void tst_strtok (void) {
//   char *tok;

//   tok = strtok (strtok_test, " \t\n");

//   while (tok != NULL) {
//     printf ("Token: %s\r\n", tok);
//     tok = strtok (NULL, " \t\n");
//   }
// }

bool startsWith(char *full, char *prefix) {
	int prefix_size = strlen(prefix);
	return strncmp(full, prefix, prefix_size);
}

bool isEqual(char *dest, char *src) {
	return strcmp(dest, src) == 0;
}

bool isEmpty(char *dest, int len) {
	int i;
	for (i = 0; i < len; ++i) {
		if (isprint(dest[i])) {
			return false;
		}
			
	}
	return true;
}
