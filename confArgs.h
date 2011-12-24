/*******************************************************/
/* confArgs.h */
/*******************************************************/

/* Captures arguments for conf application */



/* captures arguments from command line. Initial values of the variables provided are not relevant for its operation */
void  captureArguments (	int argc, char *argv[], 
	int *operation,  /* Requested mode of operation: first or second. Values defined in conf.h: enum operat {FIRST, SECOND, BOUNCE}; - in your case, BOUNCE won't be supported! */
	char *firstIp, /* For 'second' mode only: returns the requested address of node 'first' */  
	char *secondMulticastIP, /* For 'first' mode only: returns the requested MULTICAST address of node 'second' if -m option is used */
	int *port, /* Both modes: returns the port requested to be used in the communication */ 
	int *vol, /* Both modes: returns the volume requested (for both playing and recording). Value in range [0..100] */
	int *packetDuration,  /*Both modes: returns the requested duration of the playout of an UDP packet. Measured in ms */
	int *verbose, /* Both modes: returns if the user wants to show detailed traces or not */
	int *payload, /* Both modes: returns the requested payload for the communication. This is the payload to include in RTP packets. Values defined in conf.h: enum payload {PCMU=100,  L16_1=11}. Therefore, in command line either number 100 or number 11 are expected */
	int *bufferingTime /*  Both modes: returns the buffering time requested before starting playout. Time measured in ms. */
	);

/* prints current values - can be used for debugging */
void  printValues (int operation, const char * firstIp, const char * secondMulticastIP, int port, int vol, int packetDuration, int verbose, int payload, int bufferingTime );



