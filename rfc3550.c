#ifndef RFC3550_C
#define RFC3550_C

/* Modified for use in UC3M lab */

#include "rfc3550.h"
#include <sys/utsname.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>


/* ============================================================================ */
/* Do it the first time. Set 'prob' to the number of packets that must arrive in a row to declare the source as validated */
void init_seq(source *s, u_int16 seq, int prob)
{
  s->base_seq = seq;
  s->max_seq = seq;
  s->bad_seq = RTP_SEQ_MOD + 1;
  s->cycles = 0;
  s->received = 0;
  s->received_prior = 0;
  s->expected_prior = 0;
  /* other initialization */

  s->cycles = 0;
  s->transit = 0;
  s->jitter = 0;

  s->probation = prob;
}



/*==================================================================================================*/
/* RTCP */

/* ------------------------------------- */
/* CHECK VALIDITY OF RTCP PACKET */
/* returns 1 if the packet is valid, 0 otherwise */
/* UC3M: when you receive an UDP packet containing a sequence of concatenated RTCP elements (i.e. a 'SR+SDES+BYE') you can use this function to make a very basic check on the validity of the received RTCP packet.
len is the UDP data length. r is a pointer to the buffer in which the UDP packet has been copied after a 'read' operation - since your pointer to the buffer will probably be 'void *', you will need a cast operation before calling to this. For example, your code could be

result = RTCPvalidity (LEN, (rtcp_t *) buffer );
.

Then, the RTCPvalidity function checks that the length declared for each RTCP element is well defined, so that adding the size of each element you obtain the total size of the UDP packet. (numerical example: it a 'SR' element has been coded with a length of 32, a 'SDES' element with size 48, and 'BYE' element with length 24, the function returns sucess if the length of the whole packet was 104, and 0 otherwise. 
*/

int RTCPvalidity (u_int32 len , rtcp_t *r)
{
  rtcp_t *end;        /* end of compound RTCP packet */
 

  // printf ("Version: %d, padding %d, cuenta %d, pt %d, longitud %d", (r->common).version, (r->common).p, (r->common).count, (r->common).pt, (r->common).length); fflush (stdout);

  if ((ntohs (*(u_int16 *) r) & RTCP_VALID_MASK) != RTCP_VALID_VALUE) {
    printf ("something wrong with packet format\n"); fflush (stdout);
    
    return 0;
  }
  end = (rtcp_t *)((u_int32 *)r + len);
  
  do r = (rtcp_t *)((u_int32 *)r + ntohs (r->common.length) + 1) ;
  while (r < end && r->common.version == 2);
  
  if (r != end) {
    printf ("Unexpected end\n"); fflush (stdout);
    /* something wrong with packet format */
    return 0;
  }
  return 1;

}


/* ESTIMATING THE NUMBER OF LOST AND EXPECTED PACKETS */

/* Just some ideas, it is not working code */
/*

  extended_max = s->cycles + s->max_seq;
  expected = extended_max - s->base_seq + 1;

  lost = expected - s->received;
  expected_interval = expected - s->expected_prior;
  s->expected_prior = expected;
  received_interval = s->received - s->received_prior;
  s->received_prior = s->received;
  lost_interval = expected_interval - received_interval;
  if (expected_interval == 0 || lost_interval <= 0) fraction = 0;
  else fraction = (lost_interval << 8) / expected_interval;

*/

/* ideas to estimate jitter - see RFC 

       int transit = arrival - r->ts;
       int d = transit - s->transit;
       s->transit = transit;
       if (d < 0) d = -d;
       s->jitter += (1./16.) * ((double)d - s->jitter);

   When a reception report block (to which rr points) is generated for
   this member, the current jitter estimate is returned:

       rr->jitter = (u_int32) s->jitter;

   Alternatively, the jitter estimate can be kept as an integer, but
   scaled to reduce round-off error. The calculation is the same except
   for the last line:

       s->jitter += d - ((s->jitter + 8) >> 4);


   In this case, the estimate is sampled for the reception report as:

       rr->jitter = s->jitter >> 4;

 */



/* GENERATE SDES */
/*   This function builds one SDES chunk into buffer b composed of argc
     items supplied in arrays type , value and length b */
/* returns a pointer to the next byte to the last written for this packet */

char *rtp_write_sdes(char *b, u_int32 src, int argc,
			rtcp_sdes_type_t type[], /* UC3M: must be an array of 'argc' elements. Each element is an integer indicating the type of the information next (CNAME, etc.) */   
                        char *value[], /* UC3M: must be an array of 'argc' pointers to char. This is the text of each SDES element (for example, the text of the CNAME. WARNING: first you must allocate the bytes to accomodate the text. Example:
                        char myCNAME = 'put@cname.here';
                        ...
                        value[0] = myCNAME;
                        value[1] = myTOOL;
                        ...
                        */ 
                        int length[] /* UC3M this is an array of 'argc' different lengths, being length[0] the length of the data introduced in value[0], etc. */
                        )
   {
       rtcp_sdes_t *s = (rtcp_sdes_t *)b;
       rtcp_sdes_item_t *rsp;
       int i;
       int len;
       int pad;
       /* SSRC header */
       s->src = src;
       rsp = &s->item[0];
       /* SDES items */
       for (i = 0; i < argc; i++) {
           rsp->type = type[i];
           len = length[i];
           if (len > RTP_MAX_SDES) {
               /* invalid length, may want to take other action */
               len = RTP_MAX_SDES;
           }
           rsp->length = len;
           memcpy(rsp->data, value[i], len);
           rsp = (rtcp_sdes_item_t *)&rsp->data[len];
       }
       /* terminate with end marker and pad to next 4-octet boundary */
       len = ((char *) rsp) - b;
       pad = 4 - (len & 0x3);
       b = (char *) rsp;
       while (pad--) *b++ = RTCP_SDES_END;
       return b;
   }



/* -------------------------------------- */
/* PARSING SDES */
/* UC3M: simplified from original example - we know there will only be a single peer, and we only expect CNAME and TOOL info.
r is a pointer to the header of the received SDES information. 
s is a pointer to the structure which holds the RTCP information received from the peer - see rtp.h */



void rtp_read_sdes(rtcp_t *r, source *s)
{
  int count = r->common.count;
  rtcp_sdes_t *sd = &r->r.sdes;
  rtcp_sdes_item_t *rsp, *rspn;
  rtcp_sdes_item_t *end = (rtcp_sdes_item_t *)
    ((u_int32 *)r + r->common.length + 1);
  
  while (--count >= 0) {
  	  rsp = &sd->item[0];
  	  if (rsp >= end) break;

  	  for (; rsp->type; rsp = rspn ) {
  	  	  rspn = (rtcp_sdes_item_t *)((char*)rsp+rsp->length+2);
  	  	  if (rspn >= end) {
  	  	  	  rsp = rspn;
  	  	  	  break;
      }	
      /*	 UC3M specific: */
      if (rsp -> type == RTCP_SDES_CNAME) {
      	      memcpy (s->CNAME, rsp->data, rsp->length);
      } else if (rsp -> type == RTCP_SDES_CNAME) {
      	      memcpy (s->TOOL, rsp->data, rsp->length);
      } else {
      	      printf ("Unexpected SDES item type.\n"); fflush (stdout); exit (-1); 
      }	
      /* end UC3M specific */
    }
    sd = (rtcp_sdes_t *)
      ((u_int32 *)sd + (((char *)rsp - (char *)sd) >> 2)+1);
  }
  

  if (count >= 0) {
  	  printf ("Invalid packet format.\n"); fflush (stdout); exit (-1); 
      }	
    
}



#endif        //  #ifndef RFC3550_C

