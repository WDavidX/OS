#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int pid;
	int status=0;
	pid=fork();
	if (pid>0)
		{ printf ("Parent: child has pid=%dpn",pid);
			pid=wait(&status);
		}else if (pid==0)
		{ printf ("Child here\n");
			exit(status);
		}else
		{perror("fork problem\n");
			exit(-1);
			}
	return 0;
}



