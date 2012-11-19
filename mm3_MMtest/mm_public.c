/* CSci4061 F2012 Assignment 3
* section: 3
* login: schm2225
* date: 11/12/12
* names: Aaron Schmitz, Weichao Xu
* id: 3891645, 4284387
*/

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

	// error checking for MM pointer
	if (MM == NULL) {
		fprintf(stderr, "Memory manager pinter is NULL %p\n", MM);
		return -1;
	}

	// error checking for the size to be allocated
	if (tsz < 1) {
		fprintf(stderr, "Invalid input memory manager size %d\n", tsz);
		return -1;
	}

	// main stuff starts here
	// Allocate stuff here, it may be out of memory. so error ching after done
	MM->stuff = malloc(tsz);
	if (MM->stuff == NULL) {
		perror("Memory manager initialization failed. Out of memory? \n");
		return 0;
	};

	// initial some attributes and create the first node info
	MM->max_avail_size = tsz;
	MM->tsz = tsz;
	MM->partitions = 1;
	MM->first = malloc(sizeof(node));
	(MM->first)->address = MM->stuff;
	(MM->first)->inuse = FREE;
	(MM->first)->next = NULL;
	(MM->first)->prev = NULL;
	(MM->first)->size = tsz;
//	fprintf(stderr, "MM stuff %p ~ %p\n", MM->stuff, (MM->stuff + MM->tsz - 1));
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
		if (MM->stuff == NULL) {
			perror("Allocation failed. Out of memory? \n");
			return NULL;
		};
		newNode->size = neededSize;
		newNode->inuse = INUSE;
		newNode->address = currp->address;

		//both allocated and free memory partition are stored
		//build up the doubly linked list
		//sum of partition length should be the size pre-allocated

		newNode->prev = currp->prev;
		if (currp->prev != NULL)
			(currp->prev)->next = newNode;
		else
			MM->first = newNode;
		(currp)->size = (currp)->size - neededSize;
		newNode->next = currp;
		currp->prev = newNode;
		currp->address = currp->address + neededSize;
		++MM->partitions;
		return newNode->address;
	}

	if (currp == NULL) {
		fprintf(stderr, "End of memory. No space allocated. Return NULL\n");
		return NULL;
	} else {
		fprintf(stderr, "Current pointer %p is invalid.\n ", currp);
		return NULL;
	}
}

void mm_put(mm_t *MM, void *chunk) {
	if ((chunk < MM->stuff) || (chunk >= (MM->stuff + MM->tsz))) {
		fprintf(stderr, "Input pointer %p is out of range %p ~ %p\n", chunk,
				MM->stuff, (MM->stuff + MM->tsz - 1));
		return;
	}

	node *currp = MM->first;

	while (currp != NULL) {
		if (chunk > currp->address) {
			currp = currp->next;
			continue;
		}
		if (chunk < currp->address) {
			fprintf(stderr,
					"Input pointer %p is not a valid partition start. Now currp address at %p\n",
					chunk, currp->address);
			return;
		}

		if (currp->inuse == FREE) {
			fprintf(stderr,
					"Input pointer %p is a valid but the partition is not previously allocated.\n",
					chunk);
			return;
		}

		node *prevp = currp->prev, *nextp = currp->next;

		// There is no need to merge
		if ((prevp != NULL && nextp != NULL && prevp->inuse && nextp->inuse)
				|| (prevp == NULL && nextp != NULL && nextp->inuse)
				|| (nextp == NULL && prevp != NULL && prevp->inuse)) {
			currp->inuse = FREE;
			return;
		}

		// Eveytime we merge, we will get rid of partition referred by currp
		// Expand on the two sides
		if (prevp != NULL && prevp->inuse == FREE) {
			prevp->size += currp->size;
			prevp->next = nextp;
			if (prevp->next != NULL)
				(prevp->next)->prev = prevp;
			free((void*) currp);
			currp = prevp;
			--MM->partitions;
		}

		if (currp->next != NULL && nextp->inuse == FREE) {
			nextp->size += currp->size;
			nextp->prev = currp->prev;
			if (nextp->prev != NULL)
				(nextp->prev)->next = nextp;
			nextp->address = currp->address;
			free((void*) currp);
			if (nextp->prev == NULL)
				MM->first = nextp;
			--MM->partitions;
		}
		break;
	}

}

void mm_release(mm_t *MM) {
	if (MM == NULL) {
		fprintf(stderr, "Input pointer is NULL");
		return;
	}
	// Some chunks are still in use
	if (MM->partitions > 1 || MM->first->inuse == INUSE) {
		fprintf(stderr, "There are partitions in use.");
		return;
	}
	free((void*) (MM->stuff));
	free((void*) (MM->first));
}

int nodelistcheck(mm_t *MM) {
	node *currp = MM->first;
	int sum_partition = 0;
	fprintf(stderr, ">>>MM stuff(%p), firstad(%p),partition(%d), size(%d)\n",
			MM->stuff, (MM->first)->address, MM->partitions, MM->tsz);
	while (currp != NULL) {
		sum_partition += currp->size;
		fprintf(stderr, "%p (size %d, inuse %d)\t", currp->address, currp->size,
				currp->inuse);
		currp = currp->next;
	}
	fprintf(stderr, "\n");

	if (sum_partition == MM->tsz) {
		fprintf(stderr, "Total sum is correct.\n\n");
		return 1;
	} else {
		fprintf(stderr, "Total sum is WRONG %d\n\n", sum_partition);
		return 0;
	}

}
