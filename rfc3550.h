/* from RFC 3550 */
/* Modified for use in UC3M lab */

#include "rtp.h"
#include <sys/time.h>
#include <unistd.h>



/* Do it the first time. 'prob' to the number of packets that must arrive in a row to declare the source as validated
UC3M: since we are not validating sources by ensuring that a reasonable number of packets with consecutive sequence number have been received, set 'prob' to 0*/
void init_seq (source *s, u_int16 seq, int prob);


/* ------------------------------------- */
/* CHECK VALIDITY OF RTCP PACKET */
/* returns 1 if the packet is valid, 0 otherwise */
/* UC3M: when you receive an UDP packet containing a sequence of concatenated RTCP elements (i.e. a 'SR+SDES+BYE') you can use this function to make a very basic check on the validity of the received RTCP packet.
len is the UDP data length. r is a pointer to the buffer in which the UDP packet has been copied after a 'read' operation - since your pointer to the buffer will probably be 'void *', you will need a cast operation before calling to this. For example, your code could be

result = RTCPvalidity (LEN, (rtcp_t *) buffer );
.

Then, the RTCPvalidity function checks that the length declared for each RTCP element is well defined, so that adding the size of each element you obtain the total size of the UDP packet. (numerical example: it a 'SR' element has been coded with a length of 32, a 'SDES' element with size 48, and 'BYE' element with length 24, the function returns sucess if the length of the whole packet was 104, and 0 otherwise. 
*/
int RTCPvalidity (u_int32 len /* length of the whole packet in words */, rtcp_t *r  /* pointer to the start of RTCP packet*/);


/* -------------------------------------- */
/* PARSES SDES */
/* r is a pointer to the header of the received SDES information. 
s is a pointer to the structure which holds the RTCP information received from the peer - see rtp.h */

void rtp_read_sdes(rtcp_t *r, source *s);


/* Generate SDES */
/*   This function builds one SDES chunk into buffer b composed of argc
     items supplied in arrays type , value and length b */
/* returns a pointer to the next byte to the last written for this packet */
char *rtp_write_sdes(char *b, u_int32 src, int argc, rtcp_sdes_type_t type[], char *value[], int length[]);




