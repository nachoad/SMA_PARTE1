/*******************************************************/
/* confArgs.c */
/*******************************************************/

/* Captures arguments for 'conf' application */


#include "conf.h"
#include "confArgs.h"
#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h> 


void captureFirst (int argc, char *argv[], char * secondMulticastIP, int * port, int *vol, int * packetLength, int * verbose, int * bufferingTime);
void captureSecond (int argc, char *argv[], char * firstIP, int * port, int *vol, int * packetLength, int *payload, int * verbose, int * bufferingTime);


/*=====================================================================*/
void  printValues (int operation, const char * firstIP, const char * secondMulticastIP, int port, int vol,  int packetDuration, int verbose, int payload, int bufferingTime)
{
  if (operation == FIRST) { printf ("First ");}
  else if (operation == SECOND) {
    printf ("Second ");
    if (payload == PCMU)
      {
	printf (" with 8 bits, single channel, mu law\n");
      }
    else if (payload == L16_1)
      {
	printf (" with PCM 16 bits, singleChannel\n");
      }
    printf ("Address of first node: %s\n", firstIP);
    
  }



  printf (" Volume %d", vol);
  printf (" Packet duration %d\n ms", packetDuration);
  printf (" (Equivalent) time of accumulated data %d ms", bufferingTime);

  if (verbose)
    {
      printf (" Verbose mode");
    }


};



/*=====================================================================*/
void printHelp (void)
{
  printf ("\nconf first [-pPORT_RTP_FIRST] [-c] [-vVOL] [-lPACKET_DURATION] [-mMULTICAST_ADDR] [-kACCUMULATED_TIME]"
"\nconf second addressOfFirst [-pPORT_RTP_SECOND] [-c] [-vVOL] [-lPACKET_DURATION] [-kACCUMULATED_TIME] [-yPAYLOAD]\n\n(NOTA: firstName can be either a name or an IP address\n\n");
}





/* Insert default values */

/*=====================================================================*/
void  defaultValues (int *operation, char * firstIP, char * secondMulticastIP, int *port, int *vol, int *packetDuration, int *verbose, int *payload, int * bufferingTime)
{
  secondMulticastIP[0] = '\0';
  firstIP[0] = '\0';
  *port = 5004;

  (*vol) = 90;

  (*packetDuration) = 20; /* 20 ms */

  (* payload) = PCMU;
  (* verbose) = 0; 
  (* bufferingTime) = 100; /* in ms */
};



/*=====================================================================*/
void  captureArguments (int argc, char *argv[], int *operation, char * firstIP, char * secondMulticastIP, int *port, int *vol, int *packetDuration, int *verbose, int *payload, int *bufferingTime)
{

  if (argc==1)
    { printHelp ();
      exit(-1);
    }


  defaultValues (operation, firstIP, secondMulticastIP, port, vol, packetDuration, verbose, payload, bufferingTime);


  /* the first argument is always the role played by the host */
  if (strcmp ("first", argv[1]) == 0)
    {
      (*operation) = FIRST;
      if (argc > 2)
	{
	  captureFirst (argc-1, argv, secondMulticastIP, port, vol, packetDuration, verbose, bufferingTime);
	}
    }
  else if (strcmp ("second", argv[1]) == 0)
    {
      (*operation) = SECOND;
      captureSecond (argc-1, argv, firstIP, port, vol, packetDuration, payload, verbose, bufferingTime);
    }

  else 
    {
      printHelp();
      exit (-1);
    }
}



/*=====================================================================*/
void captureFirst (int argc, char *argv[], char * secondMulticastIP, int * port, int *vol, int * packetDuration, int * verbose, int * bufferingTime)
{


  int index;
  char character;

  for ( index=2; argc>1; ++index, --argc)
    {
      if ( *argv[index] == '-')
	{ 
	  character= *(++argv[index]);

	    switch (character)
	      { 
	      case 'p': /* RTP PORT for FIRST*/ 
		if ( sscanf (++argv[index],"%d", port) != 1)
		  { 
		    printf ("\nError introducing port number\n");
		    exit(-1);
		  }
 
	
		if (  !  (( (*port) >= 1024) && ( (*port) <= 65535)  ))
		  {	    
		    printf ("\nError introducing port number\n");
		    exit(-1);
		  }
		break;


	      case 'c': /* VERBOSE  */
		(*verbose) = 1;
		break;


	      
	    case 'v': /* VOLume */ 
		if ( sscanf (++argv[index],"%d", vol) != 1)
		  { 
		    printf ("\nError introducing volume\n");
		    exit(-1);
		  }
 
	
		if (  ! (( (*vol) >= 0) && ( (*vol) <= 100) ))
		  {	    
		    printf ("\nError introducing volume\n");
		    exit(-1);
		  }
		break;


	      case 'l': /* Packet duration in ms */
		if ( sscanf (++argv[index],"%d", packetDuration) != 1)
		  { 
		    printf ("\nError introducing packet duration\n");
		    exit(-1);
		  }
		
		if (  ! ( ((*packetDuration) >= 0)  ))
		  {	    
		    printf ("\nError introducing packet duration\n");
		    exit(-1);
		  }
		break;


	      case 'm': /* address of the second - can be mcast */
		if ( sscanf (++argv[index],"%s", secondMulticastIP) != 1)
		  {
		    printf ("\nError introducing multicast address\n");
		    exit(-1);
		  }
		break;


	      case 'k': /* Time accumulated in buffers */
		if ( sscanf (++argv[index],"%d", bufferingTime) != 1)
		  { 
		    printf ("\nError introducing time accumulated in buffers before starting playback\n");
		    exit(-1);
		  }
		
		if (  ! ( ((*bufferingTime) >= 0)  ))
		  {	    
		    printf ("\nError introducing time accumulated in buffers before starting playback\n");
		    exit(-1);
		  }
		break;


	      default:
		printHelp ();
		exit (-1);

	      }
					
	    }
	
     
    }
  

};



/*=====================================================================*/
void captureSecond (int argc, char *argv[], char * firstAddress, int * port, int *vol, int * packetDuration, int *payload, int * verbose, int * bufferingTime)
{

  int  maxNameNumber = 1;
  
  int index;
  int nameNumber = 0;
  char character;


  for ( index=2; argc>1; ++index, --argc)
    {
 
     if ( *argv[index] == '-')
	{
	  character= *(++argv[index]);
	  switch (character)
	    { 
	    case 'p': /* PORT */ 
	      if ( sscanf (++argv[index],"%d", port) != 1)
		{ 
		  printf ("\nError introducing the port\n");
		  exit(-1);
		}
	      
	      
	      if (  ! (( (*port) >= 1024) && ( (*port) <= 65535 )))
		{	    
		  printf ("\nError introducing the port\n");
		  exit(-1);
		}
	      break;

	    case 'c': /* VERBOSE  */
	      (*verbose) = 1;
	      break;
	      
	      
	    case 'v': /* VOLume */ 
		if ( sscanf (++argv[index],"%d", vol) != 1)
		  { 
		    printf ("\nError introducing volume\n");
		    exit(-1);
		  }
 
	
		if (  ! (( (*vol) >= 0) && ( (*vol) <= 100) ))
		  {	    
		    printf ("\nError introducing volume\n");
		    exit(-1);
		  }
		break;

	      
	    case 'l': /* Packet duration */
	      if ( sscanf (++argv[index],"%d", packetDuration) != 1)
		{ 
		  printf ("\nError introducing the duration of the packet\n");
		  exit(-1);
		}
	      
	      if (  ! ( ((*packetDuration) >= 0)  ))
		{	    
		  printf ("\nError introducing the duration of the packet\n");
		  exit(-1);
		}
	      break;
	      
	      
	     case 'k': /* Accumulated time in buffers */
		if ( sscanf (++argv[index],"%d", bufferingTime) != 1)
		  { 
		    printf ("\nError introducing time accumulated in buffers before starting playback\n");
		    exit(-1);
		  }
		
		if (  ! ( ((*bufferingTime) >= 0)  ))
		  {	    
		    printf ("\nError introducing time accumulated in buffers before starting playback\n");
		    exit(-1);
		  }
		break;


          case 'y': /* PAYLOAD INICIAL */
	      if ( sscanf (++argv[index],"%d", payload) != 1)
		{ 
		  printf ("\nError introducing the payload number\n");
		  exit(-1);
		}
	      
	      if (  ! ( ((*payload) == PCMU) || ( (*payload) == L16_1)  ))
		{	    
		  printf ("\nError introducing the payload number\n");
		  exit(-1);
		}
	      break;
	      

	      
	      
	    default:
	      printHelp ();
	      exit (-1);
	      
	      
	      
	    }
	}
   

        else /* THERE IS A NAME */
        {
          strcpy (firstAddress, argv[index]);
       
	  nameNumber += 1;
	  
	  if  (nameNumber > maxNameNumber)
	    {
	      printHelp ();
	      exit(-1);
	    }
	


        }                  
    }
  
  if (nameNumber != 1)
    {
      printHelp();
      exit (-1);
    }
  
};



	
