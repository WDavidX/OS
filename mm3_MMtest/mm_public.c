#include "mm_public.h"

/* Returns microseconds. */
double comp_time(struct timeval times, struct timeval timee) {
	double elap = 0.0;

	if (timee.tv_sec > times.tv_sec) {
		elap += (((double) (timee.tv_sec - times.tv_sec - 1)) * 1000000.0);
		elap += timee.tv_usec + (1000000 - times.tv_usec);
	} else {
		elap = timee.tv_usec - times.tv_usec;
	}
	return ((unsigned long) (elap));
}

/* Write these ... */
int mm_init(mm_t *MM, int tsz) {

//	if (MM != NULL) {
//		fprintf(stderr,"Memory manager pinter is not NULL %p\n",MM);
//		return -1;
//	}

	if (tsz < 1) {
		fprintf(stderr, "Invalid input memory manager size %d\n", tsz);
		return -1;
	}

	MM->stuff = malloc(tsz);
	if (MM->stuff == NULL) {
		perror("Memory manager initialization fails.Out of memory? \n");
		return 0;
	};

	MM->max_avail_size = tsz;
	MM->tsz = tsz;
	MM->partitions = 1;
	MM->first = malloc(sizeof(node));
	(MM->first)->address = MM->stuff;
	(MM->first)->inuse = FREE;
	(MM->first)->next = NULL;
	(MM->first)->prev = NULL;
	(MM->first)->size = tsz;
	fprintf(stderr, "MM stuff %p ~ %p\n", MM->stuff, (MM->stuff + MM->tsz - 1));
	return 1;
}

void* mm_get(mm_t *MM, int neededSize) {

	if (neededSize < 1) {
		fprintf(stderr, "Invalid neededSize %d\n", neededSize);
		return NULL;
	}

	node *currp = MM->first; // current pointer

	while (currp != NULL) {
		// skip this partition if in use
		if (currp->inuse == INUSE || currp->size < neededSize) {
			currp = currp->next;
			continue;
		}

		// if partition size is the size needed, just change status of node
		if (currp->size == neededSize) {
			currp->inuse = INUSE;
			return currp->address;
		}

		node *newNode = malloc(sizeof(node));
		newNode->size = neededSize;
		newNode->inuse = INUSE;
		newNode->address = currp->address;
		//build up the doubly linked list

		newNode->prev = currp->prev;
		if (currp->prev != NULL) {
			(currp->prev)->next = newNode;
		} else {
			MM->first = newNode;
		}

		(currp)->size = (currp)->size - neededSize;
		newNode->next = currp;
		currp->prev = newNode;
		currp->address = currp->address + neededSize;
		++MM->partitions;
		return newNode->address;
	}

	if (currp == NULL) {
		fprintf(stderr,
				"Reach end of memory located. Not space allocated. Return NULL\n");
		return NULL;
	} else {
		fprintf(stderr, "Current pointer %p is invalid.\n ", currp);
		return NULL;
	}
}

void mm_put(mm_t *MM, void *chunk) {
	if ((chunk < MM->stuff) || (chunk >= (MM->stuff + MM->tsz))) {
		fprintf(stderr, "Input pointer %p is out of range %p ~ %p", chunk,
				MM->stuff, (MM->stuff + MM->tsz - 1));
		return;
	}

	node *currp = MM->first;

	while (currp != NULL) {
		if (chunk < currp->address)
			currp = currp->next;
		if (chunk > currp->address) {
			fprintf(stderr, "Input pointer %p is not a valid partition start",
					chunk);
			return;
		}
		if (currp->inuse == FREE) {
			fprintf(stderr,
					"Input pointer %p is a valid but the partition is not previously allocated.",
					chunk);
			return;
		}

		// There is no need to merge
		if (((currp->prev->inuse) == INUSE && (currp->next)->inuse == INUSE)
				|| (currp->prev == NULL && (currp->next)->inuse == INUSE)
				|| ((currp->prev->inuse) == INUSE && currp->next == NULL)) {
			currp->inuse = FREE;
			return;
		}

		node *prevp=currp->prev, *nextp=currp->next;
		if (prevp!=NULL && prevp->inuse==FREE){
			prevp->size+=currp->size;
			prevp->next=nextp;
			if (prevp->next!=NULL)
				prevp->next->prev=prevp;
			free((void*) currp);
			currp=prevp;
		}

		if (currp->next!=NULL && nextp->inuse==FREE){
			nextp->size+=currp->size;
			nextp->prev=currp->prev;
			if (nextp->prev!=NULL)
				(nextp->prev)->next=nextp;
			free((void*) currp);
			currp=nextp;
		}

	}

}

void mm_release(mm_t *MM) {
	if (MM == NULL) {
		fprintf(stderr, "Input pointer is NULL");
		return;
	}

	if (MM->partitions > 1 || MM->first->inuse == INUSE) {
		perror("There are partitions in use.");
		return;
	}
	free((void*) (MM->stuff));
	free((void*) (MM->first));
//	free((void*) MM);
	fprintf(stderr,"Memory manager contents released. You now can release the pointer to mm_t.\n");
}

//int createNode(int nodeSize, int nodeInUse);
