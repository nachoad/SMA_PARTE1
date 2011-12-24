/*******************************************************/
/* audioSimpleArgs.c */
/*******************************************************/

/* captures arguments from command line for audioSimple */

#include "audioConfig.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h> 


/*=====================================================================*/
void printValues (int operacion, struct structSndQuality sndData, int vol, int duration, char *fileName)
{
  if (operacion == RECORD) { printf ("Recording ");}
  else if (operacion == PLAY) { printf ("Playing ");}
  else if (operacion == DUPLEX) {printf ("Storing and playing in duplex mode "); }
  else if (operacion == NO_OP) {printf ("None selected ");}
  else CALL (-1, "Error: operation not defined (imprimeValores, audioSArgs)");

  printf ("the file %s\n", fileName);

  printf ("Bits %d, stereo or mono %d, sampling frequency %d", sndData.format, sndData.channels, sndData.freq);
  printf (" Volume %d", vol);
  printf (" Duration %d\n", duration);

};



/*=====================================================================*/
void printHelp (void)
{
  printf ("\naudioSimple v2.0");
  printf ("\naudioSimple record|play|duplex [-b(8|16)] [stereo] [-vVOLUME] [-dDURATION] [-fFREQUENCY] fileName\n");

}




/*=====================================================================*/
void defaultValues (struct structSndQuality *sndData, int *vol, int *dur)
{
  sndData -> format = 8;
  sndData -> channels = 1;
  sndData -> freq = 8000;

  (*vol) = 30;

  (*dur) = 0;
};



static const int  numNombresFichMax = 1;

/*=====================================================================*/
void  captureArguments (int argc, char *argv[], int *operacion, struct structSndQuality *sndData, int *vol, int *dur, char *nombreFichero)
{
int indice;
char car;
int numNombres=0;
 int operacionDef = 0; /* 0 if no operation has been defined, 1 otherwise */

  if (argc==1)
    { printHelp ();
      exit(-1);
    }
  for ( indice=1; argc>1; ++indice, --argc)
    {
      if ( *argv[indice] == '-')
	{   

	  car = *(++argv[indice]);
     	  switch (car)
	    { 

	    case 'b': /* BITS */ 
	      if ( sscanf (++argv[indice],"%d", &(sndData->format)) != 1)
		{ 
		  printf ("\nError introducing the format of the sound (bits, coding)\n");
		  exit(-1);
		}

	      
	      if (  ! ( ((sndData -> format) != 8) || ((sndData -> format) != 16) ))
		{	    
		  printf ("\nError introducing the format of the sound (bits, coding)\n");
		  exit(-1);
		}
	      break;
	      
	    case 'v': /* VOLUMEN */
	      if ( sscanf (++argv[indice],"%d", vol) != 1)
		{ 
		  printf ("\nError introducing the volume\n");
		  exit(-1);
		}
	      
	      if (  ! ( ( (*vol) >= 0) && ((*vol) <= 100)) )
		{	    
		  printf ("\nError introducing the volume\n");
		  exit(-1);
		}
	      break;

	    case 'f': /* FRECUENCIA */ 
	      if ( sscanf (++argv[indice],"%d", &(sndData->freq)) != 1)
		{ 
		  printf ("\nError introducing the frequency\n");
		  exit(-1);
		}
	      
	      if (   (sndData -> freq) < 0)
		{	    
		  printf ("\nError introducing the frequency\n");
		  exit(0);
		}
	      break;
	      
	      
	      
	    case 'd': /* DURACION */ 
	      if ( sscanf (++argv[indice],"%d", dur) != 1)
		  { 
		    printf ("\nError introducing the duration\n");
		    exit(-1);
		  }
	      
		if (   (*dur) < 0)
		  {	    
		    printf ("\nError introducing the duration\n");
		    exit(0);
		  }
		break;
		
	    default:
	      printHelp ();
	      exit (-1);
	    }
	  
	}
    
      else /* There is a name */
	{
	  
	  if ( strcmp ( "stereo", argv[indice]) == 0)
	    sndData->channels = 2;
	  else if (strcmp ("record", argv[indice]) == 0)
	    {
	      (*operacion) = RECORD;
	      operacionDef = 1;
	    }
	  else if (strcmp ("play", argv[indice]) == 0)
	    {
	      (*operacion) = PLAY;
	      operacionDef = 1;
	    }
	  else if (strcmp ("duplex", argv[indice]) == 0)
	    {
	      (*operacion) = DUPLEX;
	      operacionDef = 1;
	    }
	  else 
	    {
	      strcpy (nombreFichero, argv[indice]);
	      numNombres += 1;

	      if  (numNombres > numNombresFichMax)
		{  
		  printHelp ();
		  exit(-1);
		}
	    }
	 
	}
    }

  if (numNombres == 0)
    {
      printHelp();
      exit (-1);
    }

  if (operacionDef == 0)
    {
      /* no operation has been selected. This is an error */
      printHelp();
      exit (-1);
    }

};

