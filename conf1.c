/* Unicast_multicast communication
THIS IS CODE for host1

compile with
	gcc -o host1 mcast_example_host1.c
execute as
	./host1
	
host1 uses an unicast local address.
host2 receives data in a multicast address. It receives from ANY sender.

host2 should be started first. Then host1. 
	packet1: host1 (unicast) -> host2(multicast)
	packet2: host2 (unicast) -> host1(unicast)
In all cases, PORT is used both as source and destination (just as stated in RFC 4961)

address of host2 is hardcoded (so host2 MUST be executed in 163.117.144.141). host1 can be executed in any host with multicast connectivity to host2 (preferrably from the same link, 163.117.144/24 ) 	
	
*/

#include "audioConfig.h"
#include "confArgs.h"
#include "conf.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "circularBuffer.h"
#include "functions.h"
#include "rtp.h"


#define MAXBUF 1036 //12 Bytes de cabezera y 1024 de Datos 
//enum operat {FIRST=1, SECOND=2, BOUNCE=3};
//enum payload {PCMU=100,  L16_1=11};
#define PORT 5000
#define GROUP "225.0.1.1"
const char message[16]= "Sent from nacho";

struct sockaddr_in local, rem, rem_uni; /* to build address/port info for local node and remote node */
int s, r;

int maximo (int a, int b){
 int max;

 if (a > b){
  max = a;
  }
 else{
  max = b;
  }

return max;
}

/*=====================================================================
// activated by Ctrol-C 
void signalHandler (int sigNum)
{

  rtcp_common_t cabecera_RTCP_BYE;
  char paqueteRCTP[25];
  cabecera_RTCP_BYE.pt=203;
  //rtcp_t

  //memcpy(paqueteRCTP,cabecera_RTCP_BYE, sizeof(cabecera_RTCP_BYE));
   
  printf ("Saliendo de la aplicacion. Mandando un paquete BYE de desconexion.\n");
  if ( (r = sendto(s, paqueteRCTP, sizeof(paqueteRCTP), 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
	perror ("sendto");
  } 
  
  exit (-1);
}*/


int main(int argc, char *argv[]) {

  char buf[MAXBUF],paqueteRTP[MAXBUF]; /* bufRecord[MAXBUF-12]to receive data from remote node */
  rtp_hdr_t * ptrARTP;
  rtp_hdr_t * ptrACabeceraRTP;
  int descriptorSnd;
  struct ip_mreq mreq; /* for multicast configuration */
  struct structSndQuality datQualitySnd;
  int elementosBuffer; /* record, play, duplex */
  void *BufferCircular;
  u_int32 numSec=0; 
  unsigned int nSeq;
  int difNumSec=0;
  int Primera=0,primer_rtp=0;
  int operation;  /* Requested mode of operation: first or second. Values defined in conf.h: enum operat {FIRST, SECOND, BOUNCE}; - in your case, BOUNCE won't be supported! */
  char firstIP[16]; /* For 'second' mode only: returns the requested address of node 'first' */    char secondMulticastIP[16]; /* For 'first' mode only: returns the requested MULTICAST address of node 'second' if -m option is used */
  int port; /* Both modes: returns the port requested to be used in the communication */ 
  int vol; /* Both modes: returns the volume requested (for both playing and recording). Value in range [0..100] */
  int packetDuration;  /*Both modes: returns the requested duration of the playout of an UDP packet. Measured in ms */
  int verbose; /* Both modes: returns if the user wants to show detailed traces or not */
  int payload; /* Both modes: returns the requested payload for the communication. This is the payload to include in RTP packets. Values defined in conf.h: enum payload {PCMU=100,  L16_1=11}. Therefore, in command line either number 100 or number 11 are expected */
  int bufferingTime; /*  Both modes: returns the buffering time requested before starting playout. Time measured in ms. */;
  int status;
  int i;
  int timestamp=0;

  socklen_t from_len; /* for recvfrom */ 


  /* we configure the signal 
  sigInfo.sa_handler = signalHandler;
  sigInfo.sa_flags = 0;  
  CALL (sigaction (SIGINT, &sigInfo, NULL), "Error installing signal"); */


   /*Caputra de argumentos*/  
   captureArguments (argc, argv, &operation, firstIP, secondMulticastIP, &port, &vol, &packetDuration, &verbose, &payload, &bufferingTime);
	
	timestamp=(1024/8)/8;
	datQualitySnd.format=8;
	datQualitySnd.channels=1;
	datQualitySnd.freq=8000;

//Calculo de timestamp
 if (payload == PCMU)
      {
	timestamp=(1024/8)/8;
	datQualitySnd.format=8;
	datQualitySnd.channels=1;
	datQualitySnd.freq=8000;
      }
 else if (payload == L16_1)
      {
	timestamp=(1024/16)/44.1;
	datQualitySnd.format=16;
	datQualitySnd.channels=1;
	datQualitySnd.freq=44100;
      }


///////////////////////////
//Si estamos en el SECOND...
///////////////////////////
if (strcmp ("second", argv[1]) == 0) {
printf("SECOND: Ha entrado en SECOND\n"); fflush (stdout);

  /* preparing bind */
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = htonl (INADDR_ANY);//INADDR_ANY

  
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  /* binding socket (to unicast local address) */
  if (bind(s, (struct sockaddr *)&local, sizeof(local)) < 0) {
    perror("bind");
    return 1;
  }


 
  /* building structure for remote address/port */
  bzero(&rem, sizeof(rem));

  rem.sin_family = AF_INET;
  rem.sin_port = htons(port);
  if (inet_aton(firstIP, &rem.sin_addr) < 0) { //secondMulticastIP
    perror("inet_aton");
    return 1;
  }

printf("SECOND:Hago SENDTO al FIRST\n"); fflush (stdout);
    /* Using sendto to send information. Since I've made a bind to the socket, the local (source) port of the packet is fixed. In the rem structure I set the remote (destination) address and port */ 
    if ( (r = sendto(s, message, sizeof(message), /* flags */ 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
	perror ("sendto");
    } else {
      buf[r] = 0;
      printf("SECOND: hace Send to al FIRST (Multicas)\n"); fflush (stdout);
    }
	


    from_len = sizeof (rem); 

  /* receives from any who wishes to send to host1 in this port */  
  if ((r = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *) &rem_uni, &from_len)) < 0) {
  	  perror ("recvfrom");
    } else {
      buf[r] = 0;
      printf("SECOND: Mensaje recibido del FIRST %s\n",buf); fflush (stdout);
    }


}// Fin del if FIRST



//////////////////////////////
//Si estamos en el FIRST...
//////////////////////////////
else if (strcmp ("first", argv[1]) == 0) {
 
  /* preparing bind */
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons(port); /*port  besides filtering, will assure that info is being sent with this PORT as local port */
  
  if (inet_aton(secondMulticastIP, &local.sin_addr) < 0) {
    perror("inet_aton");
    return 1;
  }
  
  /* creating socket */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  /* binding socket - using mcast local address */
  if (bind(s, (struct sockaddr *)&local, sizeof(local)) < 0) {
    perror("bind");
    return 1;
  }

  /* setsockopt configuration for joining to mcast group */
  if (inet_aton(secondMulticastIP, &mreq.imr_multiaddr) < 0) {
    perror("inet_aton");
    return 1;
  }
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    perror("setsockopt");
    return 1;
  }

 printf("FIRST:Esperando a recibir un send to del SECOND\n"); fflush (stdout);
  from_len = sizeof (rem); /* remember always to set the size of the rem variable in from_len */	
  if ((r = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *) &rem, &from_len)) < 0) {
    perror ("recvfrom");
    } else {
    	    buf[r] = 0; /* convert to 'string' by appending a 0 value (equal to '\0') after the last received character */
    	    printf("FIRST: Recibido mensaje y ya tenemos la direccion de SECOND\n"); fflush (stdout);
    }
    
 
    
    /* Using sendto to send information. Since I've made a bind to the socket, the local (source) port of the packet is fixed. 
    In the rem structure I have the address and port of the remote host, as returned by recvfrom */ 
    if ( (r = sendto(s, message, sizeof(message), /* flags */ 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
	perror ("sendto");
    } else {
      buf[r] = 0;
      printf("FIRST: Mandamos mensaje a la direccion multicast delSECOND\n"); fflush (stdout);
    }


}// Fin del elsif SECOND
  



////////////////////////////////////
//Aqui comienza nuestra aplicaicon
////////////////////////////////////
fd_set conjunto_lectura, conjunto_escritura;
struct timeval tiempo;
tiempo.tv_sec = 3;
tiempo.tv_usec = 0;
/* definición función select */ptrARTP = (rtp_hdr_t *) paqueteRTP;
int select(int n,
fd_set *conjunto_lectura, /*conjunto de descriptores esperando en lectura*/
fd_set *conjunto_escritura, /*conjunto de descriptores esperando en escritura*/
fd_set *conjunto_excepciones, /*conjunto de descriptores esperando una excepción*/
struct timeval *timeout /* temporizador */ );
int res;

/*Creando el Buffer Circular*/
BufferCircular = createCircularBuffer (((bufferingTime)/20)+10, MAXBUF-12);


/* soundcard configuration */
descriptorSnd = 0; /* this is required to request configSnd to open the device for the first time */

/* create snd descritor and configure soundcard to given format, frequency, number of channels. We also request setting the fragment to BUFFER_SIZE size */
configSnd(&descriptorSnd, &datQualitySnd, MAXBUF-12); 
vol = configVol (datQualitySnd.channels, operation, descriptorSnd, vol);


while(1){
	tiempo.tv_sec = 5;
	tiempo.tv_usec = 0;

	printf("--------------------------------------------------------\n\n"); fflush (stdout);
	
	/* configurar el conjunto de lectura */
	FD_ZERO(&conjunto_lectura); /* borro cualquier resto que pudiera haber en la variable */
	FD_ZERO(&conjunto_escritura); /* borro cualquier resto que pudiera haber en la variable */
	FD_SET(s, &conjunto_lectura); /* añado el descriptor número s al conjunto */
	FD_SET(descriptorSnd, &conjunto_lectura); /* añado el descriptor de la tarjeta de sonido al conjunto de lectura */
	FD_SET(descriptorSnd, &conjunto_escritura); /* añado el descriptor de la tarjeta de sonido al conjunto de lectura */


if((elementosBuffer>(bufferingTime)/20)||(Primera&&elementosBuffer>0)){
	CALL (res = select (descriptorSnd+1, &conjunto_lectura, &conjunto_escritura, NULL, &tiempo),"Fallo al llamar al select");
	Primera=1;
}
else{
	CALL (res = select ((maximo(s, descriptorSnd)+1), &conjunto_lectura, NULL, NULL, &tiempo),"Fallo al llamar al select");
}


	if (res == -1) { 
		printf("Error en el SELECT\n"); 
		fflush (stdout);
	}
		
	else if (res == 0) {//Ha vencido el TIMEOUT
		//printf("Ha vencido el TIMEOUT\n"); 
		//fflush (stdout);
		if ( (r = sendto(s, message, sizeof(message), /* flags */ 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
			perror ("sendto");
    		}   
    		else {
      			buf[r] = 0;
      			printf("Nacho envia desde hot1: %s\n",message); 
      			fflush (stdout);
    		}
	}//Else IF
			
	else {

/*Recibimos paquetes por el socket y los escribimos en el buffer circular*/
		if ( FD_ISSET (s, &conjunto_lectura) == 1){ //El select se ha desbloqueado por el descriptor 's'
			if(verbose){
				printf("+\n");fflush (stdout);
			}

			from_len = sizeof (rem); /* remember always to set the size of the rem variable in from_len */	
  			if ((r = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *) &rem_uni, &from_len)) < 0) {
    				perror ("recvfrom");
    			} 
    			else {
				//Comprobamos si es un RTCP o RTP
				
				/*if(RTCPvalidity(24,ptrARTCP)){//COMPROBAR QUE EES RTCP
					ptrARTCP = (rtcp_common_t *) buf;
					if(ptrARTCP -> pt==rtcp_type_t.RTCP_BYE){
						printf("BYE RECIBIDO, Cerrando Aplicacion\n");fflush (stdout);
						exit (-1);
					}
						

				}//IF ES RTCP

				else{*/
					if(primer_rtp==0){//Es la primera vez que recibimos un dato por el SOCKET
					ptrARTP = (rtp_hdr_t *) buf;
					nSeq = ptrARTP -> seq;
                                        memcpy(pointerToInsertData (BufferCircular),ptrARTP+12,MAXBUF-12);
					primer_rtp=1;
					elementosBuffer++;
                                	}
                                	else{// Verificar perdida de paquetes, tiempos
						ptrARTP = (rtp_hdr_t *) buf;
						if(ptrARTP -> seq==nSeq+1){// El nuevo paquete es consecutivo al anterior (1,2,3...)
							memcpy(pointerToInsertData (BufferCircular),ptrARTP+12,MAXBUF-12);
							elementosBuffer++;
							nSeq = ptrARTP -> seq;
						}
						else{
							difNumSec=(ptrARTP -> seq)-nSeq;//Calculamos la diferencia dle Numero de secuencia
							if(difNumSec>0){//Si es mayor que cero repetimos paquetes anteriores y guardo el nuevo en su posicion
								for(i=0;i<difNumSec-1;i++){
									memcpy(pointerToInsertData (BufferCircular),pointerToInsertData (BufferCircular)-1,MAXBUF-12);//-1024???
									elementosBuffer++;
								}
								memcpy(pointerToInsertData (BufferCircular),ptrARTP+12,MAXBUF-12);
								nSeq = ptrARTP -> seq;
							}
						
						}
                               		}//Else del primer RTP	
				// }//Else ES RTCP	
    			} //else del recvfrom
		 }//Descriptor S del conjunto_lectura



/*  Leemos del microfono, montamos el paquete RTP y lo enviamos por el Socket*/
		if( FD_ISSET (descriptorSnd, &conjunto_lectura) == 1){//El select se ha desbloqueado por el descriptor de la tarjeta de sonido en Lectura
      			//capturo el momento justo en el que va a leer

			ptrACabeceraRTP = (rtp_hdr_t *) paqueteRTP;
			//ptrADatosRTP = (void*)paqueteRTP+sizeof(rtp_hdr_t);
			
			status = read (descriptorSnd,ptrACabeceraRTP+12, (MAXBUF-12)); // 1, 12 o 13
			
      			if (status != MAXBUF-12)
				perror("Recorded a different number of bytes than expected");

			ptrACabeceraRTP -> ssrc = 1111;			
			ptrACabeceraRTP -> seq = numSec;
                        ptrACabeceraRTP -> ts = numSec*timestamp;
			numSec++;

	
			//y lo enviamos
			if ( (r = sendto(s, paqueteRTP, sizeof(paqueteRTP), /* flags */ 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
				perror ("sendto");
   			 } 
			else {
     				printf("Enviando paquete a conf2 \n"); fflush (stdout);
				if(verbose){
					printf(".\n");fflush (stdout);
				}
			}
		}//Descriptor de la tarjeta de sonido del conjunto_Lectura


/*  Leer del buffer circular y escribir en la tarjeta de sonido*/
		if( FD_ISSET (descriptorSnd, &conjunto_escritura) == 1){//El select se ha desbloqueado por el descSND en escritura
 			status = write (descriptorSnd, pointerToReadData (BufferCircular), MAXBUF-12); 
      			if (status != (MAXBUF-12))
				perror("Played a different number of bytes than expected");
			elementosBuffer--;

		}//Descriptor de la tarjeta de sonido del conjunto_escritura
	}//ELSE
}//While(1)
   
}  //MAIN 
