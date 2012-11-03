#include <sys/time.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#define INTERVAL 0
#define INTERVAL_USEC 800000
#define SZ 64
#define how 8

#define INUSE 1
#define FREE 0


typedef struct node node; // I am playing a trick so node has a pointer to itself

struct node{
	void* address;
	int size;
	int inuse; //We used flag to indicate whether a chunk is free or not. You can however ignore that and use another solution
	node *prev;
	node *next;
};

typedef struct {
	void *stuff;
	//node *free_list; not quite want to use this
	//node *nodeList;
	node *first;
	int tsz;
	int partitions;
	int max_avail_size;
} mm_t;

int mm_init (mm_t *MM, int tsz);
void* mm_get (mm_t *MM, int neededSize);
void mm_put (mm_t *MM, void *chunk);
void  mm_release (mm_t *MM);
double comp_time (struct timeval times, struct timeval timee);

