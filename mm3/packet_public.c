#include "mm_public.h"
#include "packet_public.h"
#include <string.h>

message_t message;     /* current message structure */
mm_t MM;               /* memory manager will allocate memory for packets */
int pkt_cnt = 0;       /* how many packets have arrived for current message */
int pkt_total = 1;     /* how many packets to be received for the message */
int NumMessages = 5;   /* number of messages we will receive */
int cnt_msg=1;         /*current message being received*/

packet_t get_packet (size) {
	packet_t pkt;
	static int which;

	pkt.how_many = size;
	which=rand()%pkt.how_many; //the order in which packets will be sent is random.
	pkt.which = which;
	if (which == 0)
		strcpy (pkt.data, "aaaaaaa\0");
	else if (which == 1)
		strcpy (pkt.data, "bbbbbbb\0");
	else if(which == 2)
		strcpy (pkt.data, "ccccccc\0");
	else if(which == 3)
		strcpy (pkt.data, "ddddddd\0");
	else
		strcpy (pkt.data, "eeeeeee\0");
	return pkt;
}


void packet_handler(int sig)
{
	packet_t pkt;
	//  fprintf (stderr, "IN PACKET HANDLER, sig=%d\n", sig);
	
	pkt = get_packet(cnt_msg); // the messages are of variable length. So, the 1st message consists of 1 packet, the 2nd message consists of 2 packets and so on..
	pkt_total = pkt.how_many;
	if(pkt_cnt==0){ // when the 1st packet arrives, the size of the whole message is allocated.
	
	} 
  
	printf("CURRENT MESSAGE %d\n",cnt_msg);

	/* insert your code here ... stick packet in memory, make sure to handle duplicates appropriately */
 
	/*Print the packets in the correct order.*/

	/*Deallocate message*/

}


int main (int argc, char **argv)
{          
	/* set up alarm handler -- mask all signals within it */

	/* turn on alarm timer ... use  INTERVAL and INTERVAL_USEC for sec and usec values */

	message.num_packets = 0;
	mm_init (&MM, 200);	
	for (j=1; j<=NumMessages; j++) {
		while (pkt_cnt < pkt_total) 
			pause();

		// reset these for next message 
		pkt_total = 1;
		pkt_cnt = 0;
		message.num_packets = 0;
		cnt_msg++;
		// anything else?

	}
	/* Deallocate memory manager */
}
