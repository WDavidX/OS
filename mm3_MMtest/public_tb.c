#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include "mm_public.h"

int main() {
//
//		printf("\nBeginning of memory testing.\n");
//
//		int *pt,*ptmm;
//		printf("pt %p has size of %lu\n",pt,sizeof(pt));
//
//		pt=malloc(100);
//		printf("pt %p has size of %lu\n",pt,sizeof(*pt));
//		pt[99]=29;
//		printf ("%d\n",pt[99]);

	int partition = 5;
	int status;
	mm_t *mymm;
	fprintf(stderr, "MM pointer generated %p\n", mymm);
	mymm = (mm_t*) malloc(sizeof(mm_t));
	printf(" ");
	status = mm_init(mymm, 2);

	mm_release(mymm);

	status = mm_init(mymm, partition);
//	nodelistcheck(mymm);

	void **ptr;
	ptr = malloc(sizeof(ptr) * partition);
	int i;
	for (i = 0; i < partition; i++) {
		ptr[i] = mm_get(mymm, 1);
//		nodelistcheck(mymm);
//		fprintf(stderr, "p[%d] = %p\n\n", i, ptr[i]);
	}

	fprintf(stderr, "==============================================\n");
	nodelistcheck(mymm);
	mm_put(mymm, ptr[2]);
//	nodelistcheck(mymm);

	mm_put(mymm, ptr[0]);
//	nodelistcheck(mymm);

	mm_put(mymm, ptr[4]);
//	nodelistcheck(mymm);

	mm_put(mymm, ptr[1]);
//	nodelistcheck(mymm);

	mm_put(mymm, ptr[3]);
//	nodelistcheck(mymm);

	mm_release(mymm);

	return 0;

}


