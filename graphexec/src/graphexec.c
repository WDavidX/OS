/*
 ============================================================================
 Name        : graphexec.c
 Author      :
 Version     :
 Copyright   :
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

// for user defined macros
#define DEFAULT_INPUT_FINE_NAME "some-graph-file.txt"
#define READ_BUFFER_SIZE 1024
#define MAX_NUM_NODE 50
#define MAX_CHILDREN_LIST_NUMBER 10

//for ’status’ variable:
#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[1024]; // prog + arguments
	char input[1024]; // filename
	char output[1024]; // filenameorward
	int children[10]; // children IDs
	int num_children; // how many children this node has
	int num_parents; // how many parents this node has
	int parentsFinished; // how many parents have finished
    int status;
	pid_t pid; // track it when it’s running
} node_t;

int makeargv(const char *s, const char *delimiters, char ***argvp);

int main(int argc, char *argv[]) {

	int i, j;
	node_t *nodes;
	nodes = (node_t*) calloc(sizeof(node_t), MAX_NUM_NODE);
	//int nodeNum=initilization(argc,&argv,&nodes)
	printf("\nCSSI 4061 Assign1 graphexec starts\n");
	for (i = 0; i < argc; ++i) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	char *inputfilename = DEFAULT_INPUT_FINE_NAME;
	if (argc >= 2) {
		inputfilename = argv[1];
		printf("Entered input file name %s\n", inputfilename);
	} else {
		printf("Use default file name %s\n", inputfilename);
	}
	FILE* finput;
	if (((finput = fopen(inputfilename, "r")) == 0)) {
		perror("Invalid master input file");
		exit(1);
	} else {
		printf("File open succeeded %s\n", inputfilename);
	}

	// Reading the node connection info from the text file
	int token_num;
	int line_number;
	char line_buffer[READ_BUFFER_SIZE];
	char** argvp;
	char** childrenlist;
	for (line_number = 0; fgets(line_buffer, sizeof(line_buffer), finput) != NULL; line_number++) {
		/* note that the newline is in the buffer */
		if (line_buffer[strlen(line_buffer) - 1] == '\n') {
			line_buffer[strlen(line_buffer) - 1] = '\0';
		}
		if (strcmp(line_buffer, "\0") == 0)
			continue; //get rid of blank lines
		//printf("==========>%4d: \t%s\n", line_number, line_buffer);
		token_num = makeargv(line_buffer, ":\n", &argvp);
		if (token_num != 4) {
			perror("Invalid number of input items");
			exit(1);
		}
		nodes[line_number].id = line_number;
        nodes[line_number].status = INELIGIBLE;
		strcpy(nodes[line_number].prog, argvp[0]);
		strcpy(nodes[line_number].input, argvp[2]);
		strcpy(nodes[line_number].output, argvp[3]);

		printf("ID: %d;\tProg:  %s ;\tInput:  %s;\tOutput:  %s;\t",
				nodes[line_number].id, nodes[line_number].prog,
				nodes[line_number].input, nodes[line_number].output);

		if (strcmp(argvp[1], "none") == 0) {
			nodes[line_number].num_children = 0;
			printf("NumChild: %d", nodes[line_number].num_children);
		} else {
			nodes[line_number].num_children = makeargv(argvp[1], " ",
					&childrenlist);
			//printf("\t|%s|\t", argvp[1]);
			printf("NumChild: %d\t Children: ", nodes[line_number].num_children);
			for (i = 0; i < nodes[line_number].num_children; ++i) {
				nodes[line_number].children[i] = atoi(childrenlist[i]);
				printf("%d ", nodes[line_number].children[i]);
			}
		}
		printf("\n");

	}
	fclose(finput);

	// INITILIZATION: get nodes with no parent and put them in a queue (FIFO)
	printf("\nCalculating inheritence\n");
	for (i = 0; i < line_number; ++i) {
		for (j = 0; j < nodes[i].num_children; ++j) {
			nodes[nodes[i].children[j]].num_parents += 1;
		}
	}

	printf("\nBegin main execution\n");
	int complete = 0;
	while (complete < line_number) {
		for (i = 0; i < line_number; ++i) {
			if (nodes[i].status == INELIGIBLE) {
				if (nodes[i].parentsFinished == nodes[i].num_parents) {
					nodes[i].status = READY;
					printf("Node %d is ready\n", i);
				}
			}
			if (nodes[i].status == READY) {
				printf("Forking node %d\n", i);
				pid_t childpid = fork();
				if (childpid == -1) {
					perror("Failed to fork");
					return 1;
				}
				else if (childpid == 0) {
					char **argv;
					int argcount = makeargv(nodes[i].prog, " ", &argv);
					if (argcount == 0)
					{
						perror("No program name");
						exit(1);
					}
					//**Set IO input and output redirection here**
					printf("Executing %s\n", argv[0]);
					execvp(argv[0], argv);
					perror("Failed to execute");
					exit(1);
				}
				else {
					nodes[i].status = RUNNING;
					nodes[i].pid = childpid;
				}
			}
			if (nodes[i].status == RUNNING) {
				int status;
				int result = waitpid(nodes[i].pid, &status, WNOHANG);
				if (result == -1) {
					perror("Failed to get status");
					exit(1);
				}
				else if (result != 0) {
					nodes[i].status = FINISHED;
					complete++;
					for (j = 0; j < nodes[i].num_children; j++) {
						nodes[nodes[i].children[j]].parentsFinished++;
					}
					printf("Node %d is complete\n", i);
				}
			}
		}
	}
	printf("\nEnd of graphexec.\n");
	return 0;
}


void initilization(int argc, char *argv[]){}

// parser a string into tokens according to delimiter given
int makeargv(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;
	snew = s + strspn(s, delimiters); /* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok(t, delimiters) != NULL) /* count the number of tokens in s */
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++)
			;

	/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1;
	}
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0)
		free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok(NULL, delimiters);
	}
	*((*argvp) + numtokens) = NULL; /* put in final NULL pointer */
	return numtokens;
}
