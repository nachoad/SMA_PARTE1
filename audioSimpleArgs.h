/*******************************************************/
/* audioSimpleArgs.h */
/*******************************************************/

/* Expected format for audioSimple execution
audioSimple record|play|duplex [-b(8|16)] [stereo] [-vVOLUME] [-dDURATION] [-fFREQUENCY] fileName
*/


#ifndef AUDIO_H
#define AUDIO_H

/* audioSimple definitions */

/* -------------------------------------------------------------------------------- */
/* INCLUDES */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/soundcard.h> 
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "audioConfig.h"



/* -------------------------------------------------------------------------------- */
/* FUNCTIONS */

/* from audioSArgs.c, for capturing arguments from command line */
void defaultValues (struct structSndQuality *datosDsp, /* structure defined in audioConfig.h, contains format, channels and sampling frequency */ 
		int *vol, /* volumen in [0..100] range */ 
		int *dur /* in seconds */
		);
void captureArguments (
		int arc,
		char *argv[],
		int *operation, /* values defined in audioConfig.h: NO_OP, RECORD, PLAY, DUPLEX, SERVER, CLIENT */
		struct structSndQuality *dataDsp, /* structure defined in audioConfig.h, contains format, channels and sampling frequency */
		int *vol, /* volumen in [0..100] range */
		int * dur, /* maximum recording/playing time, in seconds */
		char * fileName		
		);
void printHelp (void);
void printValues (int operation, struct structSndQuality datosDsp, int vol, int dur, char *fileName);



#endif /* AUDIO_H */

