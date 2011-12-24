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

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>


#define MAXBUF 256
#define PORT 5000
#define GROUP "225.0.1.1"
#define CALL(v,m) {if ( (v)==-1) {perror (m); printf ("Error number: %d, %s\n", errno, strerror (errno)); exit (-1); }};
const char message[16]= "Sent from host1";

int main(void) {
  int s, r; /* s for socket, r for storing results from system calls */
  struct sockaddr_in local, rem, rem_uni; /* to build address/port info for local node and remote node */
  char buf[MAXBUF]; /* to receive data from remote node */

  socklen_t from_len; /* for recvfrom */   

  
  /* preparing bind */
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons(PORT);
  local.sin_addr.s_addr = htonl (INADDR_ANY);

  
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
  rem.sin_port = htons(PORT);
  if (inet_aton(GROUP, &rem.sin_addr) < 0) {
    perror("inet_aton");
    return 1;
  }


/* Using sendto to send information. Since I've made a bind to the socket, the local (source) port of the packet is fixed. In the rem structure I set the remote (destination) address and port */ 
if ( (r = sendto(s, message, sizeof(message), /* flags */ 0, (struct sockaddr *) &rem, sizeof(rem)))<0) {
	perror ("sendto");
    } else {
      buf[r] = 0;
      printf("Host1: Using sendto to send data to multicast destination\n"); fflush (stdout);
    }
	

/* we do not need to fill in the 'rem' variable. 
However, we need to provide in advance the maximum amount of memory which recvfrom can use starting from the 'rem' pointer - to allow recvfrom to be sure that it does not exceed the available space. To do so, we need to provide the size of the 'rem' variable */
from_len = sizeof (rem); 

/* receives from any who wishes to send to host1 in this port */  
  if ((r = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *) &rem_uni, &from_len)) < 0) {
  	  perror ("recvfrom");
    } else {
      buf[r] = 0;
      printf("Host1: Message received from unicast address. The message is: %s\n", buf); fflush (stdout);
    }


 //Aqui comienza nuestra aplicaicon
 
fd_set conjunto_lectura;
struct timeval tiempo;
tiempo.tv_sec = 3;
tiempo.tv_usec = 0;
/* definición función select */
int select(int n,
fd_set *conjunto_lectura, /*conjunto de descriptores esperando en lectura*/
fd_set *conjunto_escritura, /*conjunto de descriptores esperando en escritura*/
fd_set *conjunto_excepciones, /*conjunto de descriptores esperando una excepción*/
struct timeval *timeout /* temporizador */ );
int res;

while(1){

tiempo.tv_sec = 5;
tiempo.tv_usec = 0;

	printf("--------------------------------------------------------\n\n"); fflush (stdout);
	/* configurar el conjunto de lectura */
	FD_ZERO(&conjunto_lectura); /* borro cualquier resto que pudiera haber en la variable */
	FD_SET(s, &conjunto_lectura); /* añado el descriptor número s al conjunto */
	CALL (res = select (s+1, &conjunto_lectura, NULL, NULL, &tiempo),"Fallo al llamar al select");
	
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
			}
			
			else {
				if ( FD_ISSET (s, &conjunto_lectura) == 1){ 
					from_len = sizeof (rem); /* remember always to set the size of the rem variable in from_len */	
  					if ((r = recvfrom(s, buf, MAXBUF, 0, (struct sockaddr *) &rem_uni, &from_len)) < 0) {
    					perror ("recvfrom");
    					} 
    					else {
    	    				buf[r] = 0; /* convert to 'string' by appending a 0 value (equal to '\0') after the last received character */
    	    				printf("Hervas me dice: %s\n", buf); 
    	    				fflush (stdout);
    					} 
					}//S
				}
	}//While(1)


  
    
}   
