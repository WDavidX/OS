#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include "mm_public.h"

int main(){
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

		int partition=5;
		int status;
		mm_t *mymm;
		fprintf(stderr,"MM pointer generated %p\n",mymm);
		mymm = (mm_t*)malloc(sizeof(mm_t));
		printf(" ");
		status=mm_init(mymm,2);

		mm_release(mymm);

		status=mm_init(mymm,partition);
		nodelistcheck(mymm);
//
//		fprintf(stderr,"MM pointer %p status %d\n",mymm,status);
//		void *p1,*p2,*p3;
//		p1=mm_get(mymm,1);
//		fprintf(stderr,"pointers p1=%p \t p2=%p \t p3=%p\n",p1,p2,p3);
//		nodelistcheck(mymm);
//
//		p2=mm_get(mymm,1);
//		fprintf(stderr,"pointers p1=%p \t p2=%p \t p3=%p\n",p1,p2,p3);
//		nodelistcheck(mymm);
//
//		p3=mm_get(mymm,1);
//		fprintf(stderr,"pointers p1=%p \t p2=%p \t p3=%p\n",p1,p2,p3);
//		nodelistcheck(mymm);

		void **ptr;
		ptr=malloc(sizeof(ptr)*partition);
		int i;
		for (i=0;i<partition;i++){
			ptr[i]=mm_get(mymm,1);
			nodelistcheck(mymm);
		}

		mm_put(mymm,ptr[1]);
		nodelistcheck(mymm);

		mm_release(mymm);


		return 0;

}

int nodelistcheck(mm_t *MM){
	node *currp=MM->first;
	int sum_partition=0;
	fprintf(stderr,"MM stuff(%p), firstad(%p),partition(%d)\n",MM->stuff,(MM->first)->address,MM->partitions);
	while (currp!=NULL){
		sum_partition+=currp->size;
		fprintf(stderr,"%p (size %d, inuse %d)\t",currp->address,currp->size,currp->inuse);
		currp=currp->next;
	}
	fprintf(stderr,"\n");

	if (sum_partition==MM->tsz){
		fprintf(stderr, "Total sum is correct.\n\n");
		return 1;
	}else{
		fprintf(stderr, "Total sum is WRONG %d\n\n",sum_partition);
		return 0;
	}

}
