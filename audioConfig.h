/*******************************************************/
/* audioConfig.h */
/*******************************************************/


#ifndef SOUND_H
#define SOUND_H


#include <netinet/in.h>

/* Macro to process system calls that return 1 in case of error */
#define CALL(v,m) {if ( (v)==-1) {perror (m); printf ("Error Number: %d, %s\n", errno, strerror (errno)); exit (-1); }};

#define BUFFER_SIZE 1024   /* Size of the buffer used to read/write data to soundcard and files. 
NOTE: in your code you should change this parameter to make it variable */


/*=====================================================================================*/
/* TYPES */
struct structSndQuality
{
  int format; /* the way data is coded - PCM with X bits... */
  int channels; /* 1 mono; 2 stereo */
  int freq; /* frecuencia de muestreo en Hz*/
};

struct structHostData
{
  char ipAddress[INET_ADDRSTRLEN];  /* to held an address plus the \0 string terminating character: "XXX.YYY.ZZZ.UUU\0" */
  /* char direccionIP[INET6_ADDRSTRLEN]; */ /* para IPv6*/
  int port;
};



/* POSIBLE OPERATIONS */
enum operations {NO_OP, RECORD, PLAY, DUPLEX, SERVER, CLIENT};



/*=====================================================================================*/
/* DEFINITION OF FUNCTIONS */
/* This function changes the configuration for the descSnd provided to the parameters stated in dataSndQuality. 
If an error is found in the configuration of the soundcard, the process is stopped and an error message reported */ 
void configSnd (int *descSnd, /* If descSnd equals to 0 when calling the function, configSnd creates and initializes a new 			
			sound descriptor, which is returned in (*descSnd) */ 
	struct structSndQuality *dataSndQuality,  /* here the requested operation quality for the soundcard is passed. The 	
			dataSndQuality->freq field is updated with the real value of the frequency configured in the sound card (which may be different than requested). */
	int fragmentSize /* fragment size requested in bytes - not need to be a power of 2. The fragment is only configured 
			if the sound descriptor is created inside the function, i.e. if the initial value of descSnd was 0. */
	);

/* configures volume for descSnd, for a given operation (store or playback). If it is stereo, it configures both channels; otherwise, only the single channel in use. 
The function returns the volume actually configured in the device after performing the operation (could be different than reported).
If an error is found in the configuration of the soundcard, the process is stopped and an error message reported. */
int configVol (int stereo, int operation, int descSnd, int vol); /* */

/* this funcion creates a new file fileName. It reads numBytesToRead bytes from descSnd and stores it in the file opened. If numBytesToRead is 0, it reads forever (in this case the application calling should handle the SIGINT signal to orderly stop the operation)
If an error is found in the configuration of the soundcard, the process is stopped and an error message reported. */   
void record (int descSnd, int numBytesToRead, const char * fileName);

/* this function opens an existing file fileName. It reads numBytesToRead bytes from file fileName, and sends it to the soundcard ; if numBytesToRead is 0, it reads forever - the application calling should handle the SIGINT signal to orderly stop the operation.
If an error is found in the configuration of the soundcard, the process is stopped and an error message reported. */
void play (int descSnd, int numBytesToWrite, const char * fileName);

/* this funcion creates a new file fileName. reads numBytesToProcess bytes from the soundcard, writes again to the soundcard (so simultaneous reading and writing to the soundcard occurs), and also writes the data read to a file fileName.
If an error is found in the configuration of the soundcard, the process is stopped and an error message reported. */
void duplex (int descSnd, int numBytesToProcess, const char *fileName);
 


#endif /* SOUND_H */


