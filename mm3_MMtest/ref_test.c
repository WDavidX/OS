#include <stdio.h>
#include <stdlib.h>
int  main(){

		char *a;		a=(char*) malloc(20);
		/* int a=5; */
		printf("a=%p \tb=%p",a,&a[1]);

		return 0;

}
