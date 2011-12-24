/*******************************************************/
/* audioConfig.c */
/*******************************************************/

#include "audioConfig.h"

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/soundcard.h>

#include <string.h> /* strerror */
#include <math.h>  /* log */



/* SOUNDCARD CONFIGURATION */
/* We separate (bits, chan number, frequency) configuration from volume configuration */

/*=====================================================================*/
void configSnd (int *descSnd, struct structSndQuality *datosQosSnd, int fragmentSize)
 {
 	 
  int frgmArg, requestedFrgmArg; /* for fragment configuration */
  
  int ioctl_arg;	
  
  if ( (*descSnd) == 0)
    {
      /* opens the audio device if it was not opened before */
      CALL ((*descSnd) = open("/dev/dsp", O_RDWR), "Error opening  /dev/dsp - check (*) if file /dev/dsp exists or (*) if other program is using /dev/dsp");
      
      /* CONFIGURING FRAGMENT SIZE */
      /* It sets the fragmen size only if it is opened for the first time. Remember that this operation MUST be done before additional configuration of the sound card */
      
      /* Argument to construct: 0xMMMMSSSS  , being MMMM fragments of size 2^SSSS */    
      requestedFrgmArg = ( log ( (double) fragmentSize) / log (2.0));
      if (requestedFrgmArg > 0x0000FFFF) /* maximum acceptable value is 0x0000FFFF - note that the soundcard may impose lower limits to this value */  
      	      { printf ("Fragment size requested is too large\n"); exit (-1); }
      requestedFrgmArg = requestedFrgmArg | 0x7fff0000 ; /* this value is used to request the soundcard not to limit the number of fragments available */
      
      frgmArg = requestedFrgmArg;
      CALL (ioctl ( (*descSnd), SNDCTL_DSP_SETFRAGMENT, &frgmArg), "Failure when setting the fragment size\n");
      if (frgmArg != requestedFrgmArg) 
      	      { printf ("Fragment size could not be set to the requested value: requested argument was %d, resulting argument is %d\n", requestedFrgmArg, frgmArg); exit (-1);}

    }




  /* WARNING: this must be done in THIS ORDER:  bits, chan number, frequency. There are some sampling frequencies that are not supported for some bit configurations... */ 
  
  ioctl_arg = (datosQosSnd->format);
  CALL( ioctl((*descSnd), SNDCTL_DSP_SETFMT, &ioctl_arg), "Error setting coding format/bit number"); /* 8 is RFC3551::L8 (Pulse Code Modulation, lineal quantifying) with 8 bits; 16 is PCM with 16 bits; see /usr/include/linux/soundcard.h for more possibilities */
  if (ioctl_arg != (datosQosSnd-> format))
    perror("It was not possible to set coding format/bit number");

  ioctl_arg = (datosQosSnd->channels);  
  CALL(ioctl((*descSnd), SNDCTL_DSP_CHANNELS, &ioctl_arg), "Error setting the number of channels");
  if (ioctl_arg != (datosQosSnd->channels))
    perror("Error setting the number of channels");

  ioctl_arg = (datosQosSnd->freq);	   
  CALL (ioctl((*descSnd), SNDCTL_DSP_SPEED, &ioctl_arg), "Error setting the sampling frequency");
  datosQosSnd -> freq = ioctl_arg;
  /* we don't check the returning values, since it can be different without being an error */
 }




/*=====================================================================*/
/*  
For stereo, I configure both channels with the same volume. */
/* returns the actual volume set in the soundcard after performing the operation */
int configVol (int channels, int operation, int descSnd, int vol)
{
  int volFinal, volLeft, volRight;


  volFinal = volLeft = volRight = vol;

  if (channels == 2)
    {
      volFinal = (volRight << 8) + volLeft; 
    }


  /* playing / recording have different configuratino knobs */
  if (operation == RECORD)
    {
      CALL (ioctl (descSnd, MIXER_WRITE (SOUND_MIXER_MIC), &volFinal), "Error when seting volume for MIC");
    }
  else if (operation == PLAY)
    {
      CALL (ioctl (descSnd, MIXER_WRITE (SOUND_MIXER_PCM), &volFinal), "Error when seting volume for playing");
    }
  else if (operation == DUPLEX)
    {
      CALL (ioctl (descSnd, MIXER_WRITE (SOUND_MIXER_MIC), &volFinal), "Error when seting volume for MIC");
      CALL (ioctl (descSnd, MIXER_WRITE (SOUND_MIXER_PCM), &volFinal), "Error when seting volume for playing");
    }


  /* we assume both channels have been equally configured; we take one to return the resulting volume */
  volLeft = volFinal & 0xff;
  
  return volLeft;
}




/*=====================================================================*/
void record (int descSnd, int numBytesToRead, const char * fileName)
{
  
  /* buffer for data */
  unsigned char buf[BUFFER_SIZE];
  
  int file;
  int cycles = 0;
  int maxCycles = numBytesToRead / BUFFER_SIZE;
  int status;
  
  
  /* open file for writing */
  CALL (file = open  (fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU), "Error creating file for writing");


  /* If you need to store in the file the audio format ('Choice 1') of the subsequent data, this could be the place */
  
  printf("Storing... :\n");
  
  while ( (numBytesToRead == 0) || (cycles < maxCycles)) 
    { /* until duration, or Ctrl-C */
      /* reading blocks until there are enough data */
      status = read (descSnd, buf, sizeof(buf)); 
      if (status != sizeof(buf))
	perror("Recorded a different number of bytes than expected");
      printf (".");fflush (stdout);
      
      

      status = write (file, buf, sizeof(buf));
      if (status != sizeof(buf))
	perror ("Written in file a different number of bytes than expected"); 
      
      cycles ++;
    }
  
  printf("Recording finished.\n");
  exit (0);
}





/*=====================================================================*/
void play (int descSnd, int numBytesToWrite, const char * fileName)
{
  
  unsigned char buf[BUFFER_SIZE];
  
  int file;
  int cycles = 0;
  int maxCycles = numBytesToWrite / BUFFER_SIZE;
  int status;
  
  /* Abre fichero para lectura */
  CALL (file = open (fileName, O_RDONLY), "File could not be opened");

  /* If you need to read from the file and process the audio format ('Choice 1') of the subsequent data, this could be the place */
  
  printf("Playing... :\n");
  
  while ( ! (((maxCycles != 0) && (cycles > maxCycles)))) 
    { 
      status = read (file, buf, sizeof(buf));
      if (status != sizeof(buf))
	break; /* the file finished */
      
      
      status = write (descSnd, buf, sizeof(buf)); 
      if (status != sizeof(buf))
	perror("Played a different number of bytes than expected");
          
      cycles ++;
    }
  printf("Playing finished.\n");
  exit (0);
};





/*=====================================================================*/
/* read data from soundcard, writes again to the soundcard, and also to a file */
void duplex (int descSnd, int numBytesToProcess, const char * fileName)
{
  
  /* Buffer para las muestras */
  unsigned char buf[BUFFER_SIZE];
  
  int file;
  int cycles = 0;
  int maxCycles = numBytesToProcess / BUFFER_SIZE;
  int status;
  int ioctl_arg = 0;

  /* pone la tarjeta en modo duplex */
  CALL( ioctl(descSnd, SNDCTL_DSP_SETDUPLEX, &ioctl_arg), "Error setting duplex mode");  

  
  /* Abre fichero para escritura */
  CALL (file = open  (fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU), "Error creating file for writing");
  
  printf("Playing and storing in duplex mode... :\n");
  
  while ( (numBytesToProcess == 0) || (cycles < maxCycles)) 
    { 
      status = read (descSnd, buf, sizeof(buf)); 
      if (status != sizeof(buf))
      perror("Error: number of bytes recorded different than expected");
      printf (".");fflush (stdout);
      
      status = write (descSnd, buf, sizeof(buf)); 
      if (status != sizeof(buf))
	      perror("Error: number of bytes played different than expected");
      printf (".");fflush (stdout);
      

      status = write (file, buf, sizeof(buf));
      if (status != sizeof(buf))
	perror ("Error: number of bytes written to the file different than expected"); 
      
      cycles ++;
    }
  
  printf("Finished duplex mode.\n");
  exit (0);
}











