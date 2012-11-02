#include "mm_public.h"

/* Returns microseconds. */
double comp_time (struct timeval times, struct timeval timee)
{
	double elap = 0.0;

	if (timee.tv_sec > times.tv_sec) {
		elap += (((double)(timee.tv_sec - times.tv_sec -1))*1000000.0);
		elap += timee.tv_usec + (1000000-times.tv_usec);
	}
	else {
		elap = timee.tv_usec - times.tv_usec;
	}
	return ((unsigned long)(elap));
}

/* Write these ... */
int mm_init (mm_t *MM, int tsz){

}

void* mm_get (mm_t *MM, int neededSize) {

}

void mm_put (mm_t *MM, void *chunk) {

}

void mm_release (mm_t *MM) {
	
}
