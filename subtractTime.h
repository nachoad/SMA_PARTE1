/*******************************************************/
/* subtractTime.h */
/*******************************************************/

#include <sys/time.h>

/* Computes 
	'result' = 'first' - 'second' 
	returns value in 'result' struct
Only works if first is GREATER TIME (later) than second (and result is positive). 
If result is possitive or 0, returns 0. Otherwise, returns -1, and the contents of result are NOT VALID */
int subtractTime (struct timeval *result, const struct timeval *first, const struct timeval *second);
