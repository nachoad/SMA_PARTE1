/*
audioSimple record|play|duplex [-b(8|16)] [stereo] [-vVOLUME] [-dDURATION] [-fFREQUENCY] fileName

Operations:
- store: reads from sound card and writes to a file
- play: the opposite to 'store'
-duplex: sets soundcard to duplex mode,  writes to a file the recorded sound (and also plays it)

-b 8 or 16 bits per sample
VOL volume [0..100]
DUR duration in seconds; 0 or unspecified means that Ctrol-C is the only way to stop
FRE sampling frequency in Hz

default values:  8 bits, vol 30, dur 0, sampling frequency 8000, mono

buffer size depends on constant BUFFER_SIZE (defined in audioConfig.h)

*/



/* -------------------------------------------------------------------------------- */
/* INCLUDES */

#include "audioSimpleArgs.h"
#include "audioConfig.h"



/* -------------------------------------------------------------------------------- */
/* CONSTANTS */


const int MaxLongSize = 100; /* max filename size */



/*=====================================================================*/
/* activated by Ctrol-C */
void signalHandler (int sigNum)
{
  printf ("audioSimple was requested to finish\n");
  exit (-1);
}



/*=====================================================================*/
int main(int argc, char *argv[])
{
  struct sigaction sigInfo; /* signal conf */

  struct structSndQuality datQualitySnd;
  int vol;
  int duration, numBytes;
  char nombFich[MaxLongSize];
  int operation; /* record, play, duplex */
  int descriptorSnd;

  /* we configure the signal */
  sigInfo.sa_handler = signalHandler;
  sigInfo.sa_flags = 0;  
  CALL (sigaction (SIGINT, &sigInfo, NULL), "Error installing signal"); 

  
  defaultValues (&datQualitySnd, &vol, &duration);
  captureArguments (argc, argv, &operation, &datQualitySnd, &vol, &duration, nombFich);


  /* soundcard configuration */
  descriptorSnd = 0; /* this is required to request configSnd to open the device for the first time */

  /* create snd descritor and configure soundcard to given format, frequency, number of channels. We also request setting the fragment to BUFFER_SIZE size */
  configSnd (&descriptorSnd, &datQualitySnd, BUFFER_SIZE); 
  vol = configVol (datQualitySnd.channels, operation, descriptorSnd, vol);
   

  /* obtained values -may differ slightly - eg. frequency - from requested values */
  printValues (operation, datQualitySnd, vol, duration, nombFich);
  printf ("Duration of each packet exchanged with the soundcard :%f\n", (float) BUFFER_SIZE / (float) ((datQualitySnd.channels) * (datQualitySnd.format /8) * (datQualitySnd.freq)));


  numBytes = duration * (datQualitySnd.channels) * (datQualitySnd.format /8) * (datQualitySnd.freq); /* note that this is the actual frequency provided by the device */


  if (operation == RECORD)
    record (descriptorSnd, numBytes, nombFich);
  else if (operation == PLAY)
    play (descriptorSnd, numBytes, nombFich);
  else if (operation == DUPLEX)
    duplex (descriptorSnd, numBytes, nombFich);

  close (descriptorSnd);

  return 0;
};
 


