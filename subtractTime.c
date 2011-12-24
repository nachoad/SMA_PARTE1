/*******************************************************/
/* subtractTime.c */
/*******************************************************/

#include "subtractTime.h"
#include <stdio.h>

#define USEC_PER_SEC 1000000

/* Computes 
	'result' = 'first' - 'second' 
	returns value in 'result' struct
Only works if first is GREATER TIME (later) than second (and result is positive). 
If result is possitive or 0, returns 0. Otherwise, returns -1, and the contents of result are NOT VALID */


int subtractTime (struct timeval *result, const struct timeval *first, const struct timeval *second)
{
  struct timeval first_aux;
  

  /* Format check: assure that usec does not exceed 1000000 in any of the variables */
  if (  (first->tv_usec >= USEC_PER_SEC) || (second->tv_usec >= USEC_PER_SEC) ) {
	  printf ("Exceeded maximum acceptable value for usec field of timeval variable");
	  return (-1);
  }
  /* Check if second is greater than first, and return if this is the case */
  if (
	  ( second->tv_sec > first -> tv_sec) ||
	  ( (second -> tv_sec == first -> tv_sec) && (second -> tv_usec > first -> tv_usec) )     )
  {return -1; }

  

  /* everything is fine, so just subtract */
  if ( (first->tv_usec) < (second->tv_usec))    { 
	  first_aux.tv_usec = first->tv_usec + USEC_PER_SEC;
	  first_aux.tv_sec = first->tv_sec - 1;
      
	  /* Trick: if you add 1000000 to the usec part and subtract 1 to the sec part, it is the same value - well, in a strange format
	  eg:  3sec, 2013 usec 	is equivalent to 2 sec, 1002013 usec
	  
	  With this trick, I assure that tv_usec in first is GREATER than tv_usec in second
	  */	  
	  result->tv_sec = first_aux.tv_sec - second->tv_sec;
	  result->tv_usec = first_aux.tv_usec - second->tv_usec;
    }
    else {
	  result->tv_sec = first->tv_sec - second->tv_sec;
	  result->tv_usec = first->tv_usec - second->tv_usec;
    }
  
 
  
  return (0);
}
