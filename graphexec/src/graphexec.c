/*login: schm2225
* date: 10/1/12
* name: Aaron Schmitz, Weichao Xu
* id: 3891645, 4284387
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
#define READ_BUFFER_SIZE 1024
#define MAX_NUM_NODE 50
#define MAX_CHILDREN_LIST_NUMBER 10
#define INPUT_FLAGS O_RDONLY
#define OUTPUT_FLAGS O_WRONLY | O_CREAT
#define CREATE_FLAGS S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

//for ’status’ variable:
#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[1024]; // prog + arguments
	char input[1024]; // filename
	char output[1024]; // filename
	int children[10]; // childrens' IDs
	int num_children; // how many children this node has
	int num_parents; // how many parents this node has
	int parentsFinished; // how many parents have finished
	int status; // status of the node - ineligible, ready, running, or fininshed from above
	pid_t pid; // track it when it’s running
} node_t;

int createTree(FILE* finput, node_t *nodes);
void execute(node_t *nodes, int count);
void executeChild(node_t node);
int makeargv(const char *s, const char *delimiters, char ***argvp);

/**
 * Read in the input file specified in the arguments
 * Parse it for nodes
 * Start with the root nodes, and execute each node once its parents have finished
*/
int main(int argc, char *argv[]) {
	printf("CSSI 4061 Assignment 1\n");

	// Allocate memory for the maximum number of nodes
	node_t *nodes;
	nodes = (node_t*) calloc(sizeof(node_t), MAX_NUM_NODE);
	if (nodes == NULL)
	{
		perror("Could not allocate memory");
		exit(1);
	} else {
		printf("Memory Allocated\n");
	}

	// Parse the arguments for the input file name
	char *inputfilename;
	if (argc == 2) {
		inputfilename = argv[1];
	} else {
		printf("Invalid number of arguments\n");
		exit(1);
	}

	// Open the file
	FILE* finput;
	if (((finput = fopen(inputfilename, "r")) == 0)) {
		perror("Invalid master input file");
		exit(1);
	} else {
		printf("File open succeeded %s\n", inputfilename);
	}

	// Reading the node connection info from the text file
	printf("Begin creating node tree\n");
	int count = createTree(finput, nodes);
	fclose(finput);

	// Run the main loop that spawns and collects children
	printf("Begin main execution\n");
	execute(nodes, count);

	free(nodes);
	printf("End of graphexec\n");
	return 0;
}

/**
 * Parse the file into an array
 *
 * Read a line from the file
 * If the node is ineligible or complete, continue
 * If the node is ready, execute
 * If the node is marked as running, verify that it is still running
 * Once all nodes have completed, return
 *
 * Params: file pointer, array of nodes
 */
int createTree(FILE* finput, node_t *nodes) {
	int token_num, line_number, i, j;
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

		if (strcmp(argvp[1], "none") == 0) {
			nodes[line_number].num_children = 0;
		} else {
			nodes[line_number].num_children = makeargv(argvp[1], " ", &childrenlist);
			for (i = 0; i < nodes[line_number].num_children; ++i) {
				nodes[line_number].children[i] = atoi(childrenlist[i]);
			}
		}
	}

	for (i = 0; i < line_number; ++i) {
		for (j = 0; j < nodes[i].num_children; ++j) {
			nodes[nodes[i].children[j]].num_parents += 1;
		}
	}

	return line_number;
}

/**
 * MAIN LOOP FOR THE PROGRAM
 *
 * Repeatedly loop through each node
 * If the node is ineligible or finished, continue
 * If the node is ready, execute
 * If the node is marked as running, verify that it is still running
 * Once all nodes have completed, return
 *
 * Params: array of nodes, length of array
 */
void execute(node_t *nodes, int count) {
	int complete = 0, i, j;
	while (complete < count) {
		for (i = 0; i < count; ++i) {
			if (nodes[i].status == INELIGIBLE) {
				// If it's ready, mark it
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
					exit(1);
				}
				else if (childpid == 0) {
					executeChild(nodes[i]);
				}
				else {
					nodes[i].status = RUNNING;
					nodes[i].pid = childpid;
				}
			}
			if (nodes[i].status == RUNNING) {
				// Check the if this process has finished
				int result = waitpid(nodes[i].pid, NULL, WNOHANG);
				if (result == -1) {
					perror("Failed to get status");
					exit(1);
				}
				else if (result != 0) {
					// It has
					nodes[i].status = FINISHED;
					complete++;
					// Increment its childrens' counter so they can run
					for (j = 0; j < nodes[i].num_children; j++) {
						nodes[nodes[i].children[j]].parentsFinished++;
					}
					printf("Node %d is complete\n", i);
				}
			}
		}
	}
}

/**
 * Performs required executions for a child
 *
 * Redirect output and input
 * Execute the specified program
 *
 * Params: node corresponding to child
 */
void executeChild(node_t node) {
	char **argv;
	// Parse arguments
	int argcount = makeargv(node.prog, " ", &argv);
	if (argcount == 0)
	{
		perror("No program name");
		exit(1);
	}

	// Redirect standard input
	printf("Executing %s\n", argv[0]);
	int finput, foutput;
	if ((finput = open(node.input, INPUT_FLAGS)) == 0) {
		perror("Invalid input file");
		exit(1);
	}
	if (dup2(finput, STDIN_FILENO) == -1)
	{
		perror("Could not redirect stdin");
		exit(1);
	}

	// Redirect standard output
	if ((foutput = open(node.output, OUTPUT_FLAGS, CREATE_FLAGS)) == 0) {
		perror("Invalid output file");
		exit(1);
	}
	if (dup2(foutput, STDOUT_FILENO) == -1)
	{
		perror("Could not redirect stdout");
		exit(1);
	}

	// Execute the specified program
	execvp(argv[0], argv);
	perror("Failed to execute");
	exit(1);
}

/**
 * Parse a string into tokens according to delimiter given
 * Copied from Robbins and Robbins p. 37
 *
 * Params: string, string of delimiters, pointer that will be populated
 */
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
