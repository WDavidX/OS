//#include "mm_public.h"
#include "packet_public.h"
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

message_t message; /* current message structure */
mm_t MM; /* memory manager will allocate memory for packets */
int pkt_cnt = 0; /* how many packets have arrived for current message */
int pkt_total = 1; /* how many packets to be received for the message */
int NumMessages = 5; /* number of messages we will receive */
int cnt_msg = 1; /*current message being received*/

#define TIMER_TV_USEC 100
#define TIMER_TV_SEC 0

packet_t get_packet(int size);
void packet_handler(int sig);

int main(int argc, char **argv) {
	/* set up all the sigsets here*/

	// Weichao's Question, do we need to put all signals within it?
	// should we use static type here?
	sigset_t new_set;
	if ((sigemptyset(&new_set) == -1) || (sigfillset(&new_set) == -1)) {
		perror("Failed to set up signal set to include all signals.\n");
		exit(1);
	} else {
		if (sigprocmask(SIG_BLOCK, &new_set, NULL) == -1) {
			perror("Failed to block the incoming signals");
			exit(1);
		}
	}

	/* set up alarm handler -- mask all signals within it */
	// save the old signal action in case we messed up, comment out later
	struct sigaction old_action;
	if (sigaction(SIGALRM, NULL, &old_action) == -1) {
		perror("Unable to save the old signal action handler for SIGALRM.");
		exit(1);
	}

	// set up the new action for SIGALRM
	struct sigaction new_action;
	new_action.sa_handler = packet_handler;
	new_action.sa_flags = 0;
	if (sigaction(SIGALRM, &new_action, NULL) == 1) {
		perror("Fail to set the new action mask for SIGALRM");
	}

	/* turn on alarm timer ... use  INTERVAL and INTERVAL_USEC for sec and usec values */
	struct itimerval interval;
	interval.it_value.tv_sec=TIMER_TV_SEC;
	interval.it_value.tv_usec=TIMER_TV_USEC;
	interval.it_interval.tv_sec=TIMER_TV_SEC;
	interval.it_interval.tv_usec=TIMER_TV_USEC;
	if (setitimer(ITIMER_REAL, &interval, NULL) == -1) {
		fprintf(stderr, "Fail to set real-time timber to sec=%d, usec=%d: %s\n",
				TIMER_TV_SEC, TIMER_TV_USEC, strerror(errno));
		exit(1);
	}


	int j;
	message.num_packets = 0;
	mm_init(&MM, 200);
	for (j = 1; j <= NumMessages; j++) {
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

packet_t get_packet(int size) {
	packet_t pkt;
	static int which;

	pkt.how_many = size;
	which = rand() % pkt.how_many; //the order in which packets will be sent is random.
	pkt.which = which;
	if (which == 0)
		strcpy(pkt.data, "aaaaaaa\0");
	else if (which == 1)
		strcpy(pkt.data, "bbbbbbb\0");
	else if (which == 2)
		strcpy(pkt.data, "ccccccc\0");
	else if (which == 3)
		strcpy(pkt.data, "ddddddd\0");
	else
		strcpy(pkt.data, "eeeeeee\0");
	return pkt;
}

void packet_handler(int sig) {
	packet_t pkt;
	fprintf(stderr, "IN PACKET HANDLER, sig=%d\n", sig);

	pkt = get_packet(cnt_msg); // the messages are of variable length. So, the 1st message consists of 1 packet, the 2nd message consists of 2 packets and so on..
	pkt_total = pkt.how_many;
	if (pkt_cnt == 0) { // when the 1st packet arrives, the size of the whole message is allocated.

	}

	printf("CURRENT MESSAGE %d\n", cnt_msg);

	/* insert your code here ... stick packet in memory, make sure to handle duplicates appropriately */

	/*Print the packets in the correct order.*/

	/*Deallocate message*/

}

