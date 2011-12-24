/*******************************************************/
/* conf.h */
/*******************************************************/


#ifndef SIMPLECONF_H
#define SIMPLECONF_H

#include <sys/time.h>
#include <unistd.h>

/* -------------------------------------------------------------------------------- */
/* DEFINITIONS */
enum operat {FIRST, SECOND, BOUNCE};
enum payload {PCMU=100,  L16_1=11};

#define MaxNameLength 100


//#define CALL(v,m) {if ( (v)==-1) {perror (m); printf ("Error number: %d, %s\n", errno, strerror (errno)); exit (-1); }};


#define MaxLongBuffer 16384

/* read 'rtp.h', which defines structures that complement the following one */ 

/* struct that can be use to reflect the state of RTP (allowing interaction among the RTP and RTCP parts of your code */
struct RTPparameters {
  unsigned int payload;
  unsigned int changePayload; /* set to 1 by RTCP if according to the received RTCP info, it is required to change (reducing the quality) of the payload. RTP code can check its value, to actually perform the required change */ 
  int dataPacketSize; /* size of the data packet to be generated. Depends on the payload (and changes if payload changes) */
  int samplesPerPacket;
  int playbackTimePerPacket; /* in miliseconds */
  
  int remoteSSRC, localSSRC;
  int localSequenceNumber; /* the remote sequence number is updated by a "source" struct defined in rtp.h */ 
  
  int remoteInitialTimestamp, localInitialTimestamp, localTimestamp; 
  
  struct timeval remoteInitialTime; /* 'real time' instant at which the first packet was received from the other side */ 
  struct timeval localInitialTime; /* 'reat time' instant at which recording started in the local host. */
  int numberSentPackets, numberSentOctet; /* required for RTCP statistics */ 

};



#endif /* SIMPLE_H */



