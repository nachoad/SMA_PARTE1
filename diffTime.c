/* diffTime.c
To compile you need 'subtractTime.h' and 'subtractTime.c' in the same folder. Then...

gcc -c -o subtractTime.o subtractTime.c
gcc -c -o diffTime.o diffTime.c
gcc -o diffTime diffTime.o subtractTime.o


Examples of execution

strace -tt [...] 2>&1 | grep 'write(5' | ./ diffTime
	shows in the screen the time interval between sucessive writing operations to device descriptor #5
*/


#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "subtractTime.h"

main () {

	struct timeval first, second, diff;
	int hours, minutes;
	char s_first[1000], s_second[1000];
	int result;
	
	// Read the first two lines
	// the format of the data from strace is	 	15:24:26.607645
	// pass hours:minutes... to a 'struct timeval' variable in which seconds, which may be very large (more than 60) 
	result = scanf ("%d:%d:%d.%d %[^\n]", &hours, &minutes, (int *) &(first.tv_sec), (int *) &(first.tv_usec), s_first);
		
	if (result!= 5) {
		exit (-1); }  // it could not read the 5 expected variables 
	first.tv_sec = first.tv_sec + minutes * 60 + hours * 60*60; 
	
	result = scanf ("%d:%d:%d.%d %[^\n]", &hours, &minutes, (int *) &(second.tv_sec), (int *) &(second.tv_usec), s_second);
	

	if (result!= 5) {	
		exit (-1); }
	second.tv_sec = second.tv_sec + minutes * 60 + hours * 60*60;
	
	
	while (1) { // the loop stops when an EOF 
		result = subtractTime (&diff, &second, &first);
		if (result == -1) { 
			printf ("Unexpected result: subtraction is negative!\n");
			exit(-1);
		}
		printf ("%d.%6d:    %s\n", (int) diff.tv_sec, (int) diff.tv_usec, s_first);
		
		// move data from second to first
		first.tv_sec = second.tv_sec; first.tv_usec = second.tv_usec;
		strcpy (s_first, s_second);
		
		// read new data for second
		 
		result = scanf ("%d:%d:%d.%d %[^\n]", &hours, &minutes, (int *) &(second.tv_sec), (int *) &(second.tv_usec), s_second);

		if (result!=5) {
			if (result == EOF) {
			exit (0); } // this is assumed normal termination of the code
			else {printf ("Unexpected format\n");
				exit (-1);
				}
		}
				
		
		second.tv_sec = second.tv_sec + minutes * 60 + hours * 60*60;
	}
		
}	
		

