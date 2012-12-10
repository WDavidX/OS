/* CSci4061 F2012 Assignment 5
section: 3
login: schm2225
date: 12/12/12
names: Aaron Schmitz, Weichao Xu
id: 3891645, 4284387 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "util.h"

extern int qlen;
static int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_HTTP_OUTGOING_HEADER_SIZE 1000

/**********************************************
 * init
 - port is the number of the port you want the server to be
 started on
 - initializes the connection acception/handling system
 - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
 but exactly one time, in the main thread of your program)
 BEFORE USING ANY OF THE FUNCTIONS BELOW
 - if init encounters any errors, it will call exit().
 ************************************************/
enum boolean {
	FALSE, TRUE
};
void init(int port) {
	// set up sockets using bind, listen
	// also do setsockopt(SO_REUSEADDR)
	// exit if the port number is already in use

	// socket init
	if ((master_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Fail to initilize socket for master. ");
		exit(1);
	}

	// address reuse
	int enable = 1;

	setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &enable,
			sizeof(enable));
	//int setsockopt(int sockfd, int level, int optname,
	//				 const void *optval, socklen_t optlen);

	// socket addr
	struct sockaddr_in ma;
	ma.sin_family = AF_INET;
	ma.sin_port = htons(port);
	ma.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	if ((bind(master_fd, (struct sockaddr *) &ma, sizeof(ma))) < 0) {
		fprintf(stderr, "Binding error for %d:%d.\n", (int) ma.sin_addr.s_addr,
				port);
		perror("");
		exit(1);
	}

	if (listen(master_fd, qlen) < 0) {
		perror("Failure upon listen().");
		exit(1);
	}
}

/**********************************************
 * accept_connection - takes no parameters
 - returns a file descriptor for further request processing.
 DO NOT use the file descriptor on your own -- use
 get_request() instead.
 - if the return value is negative, the thread calling
 accept_connection must exit by calling pthread_exit().
 ***********************************************/
int accept_connection(void) {

	// accept one connection using accept()
	// return the fd returned by accept()

	// Your code goes in this function.

	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	int newsock;

	newsock = accept(master_fd, (struct sockaddr *) &cli_addr,
			(socklen_t*) &cli_addr_len);

	return newsock;

}

/**********************************************
 * get_request
 - parameters:
 - fd is the file descriptor obtained by accept_connection()
 from where you wish to get a request
 - filename is the location of a character buffer in which
 this function should store the requested filename. (Buffer
 should be of size 1024 bytes.)
 - returns 0 on success, nonzero on failure. You must account
 for failures because some connections might send faulty
 requests. This is a recoverable error - you must not exit
 inside the thread that called get_request. After an error, you
 must NOT use a return_request or return_error function for that
 specific 'connection'.
 ************************************************/
int get_request(int fd, char *filename) {
	// read from the socketfd and parse the first line for the GET info
	// if it isn't a GET request, then just dump it and return -1.
	// otherwise, store the path/filename into 'filename'.
	char buf[MAX_BUF_SIZE];
	int nbyte = read(fd, (void*) buf, MAX_BUF_SIZE);
	if (nbyte < 0) {
		perror("get_reqeust fails to read.");
		return -1;
	} else {
		fprintf(stderr, "\nNew request: %d bytes read from fd %d\n", nbyte, fd);
	}
	fprintf(stderr, "%s\n", buf);

	char **lines;
	makeargv(buf, "\n", &lines);
//	fprintf(stderr, "\n--> %s", lines[0]);

	// Syntax check for GET request
	char **line_tokens;
	makeargv(lines[0], " ", &line_tokens); // GET all cap
	if (strcmp(line_tokens[0], "GET") != 0) {
		fprintf(stderr, "Bad format, expecting GET: %s.\n", line_tokens[0]);
		return -1;
	}

	char **http_tokens;
	int nhttp = makeargv(line_tokens[2], "/", &http_tokens);
	if (nhttp != 2 || strcmp(http_tokens[0], "HTTP")) {
		fprintf(stderr, "Bad format, expecting HTTP/: %s/   %d\n",
				http_tokens[0], nhttp);
		return -1;
	}
//	fprintf(stderr, "Http token 1: %s \ token 2: %s", http_tokens[0],
//			http_tokens[1]);

	char **version_tokens;
	int nversion = makeargv(http_tokens[1], ".", &version_tokens);
	if (nversion < 1) {
		fprintf(stderr, "Bad format, expecting a decimal number: %s/   %d\n",
				http_tokens[1], nversion);
		return -1;
	}

	int ct;
//	fprintf(stderr, "Version %s\n", http_tokens[1]);

	int decimalshown = 0;
	for (ct = 0; ct < strlen(http_tokens[1]); ++ct) {
		if (http_tokens[1][ct]=='\0'||http_tokens[1][ct]==13 ) break;
		if (http_tokens[1][ct] == '.') {
			if (decimalshown == 0) {
				decimalshown = 1;
				continue;
			} else {
				fprintf(stderr, "A. decimal number is expected %s\n",
						http_tokens[1]);
				return -1;
			}
		}
//		fprintf(stderr,"%c %d %d %d\n",http_tokens[1][ct],'0',http_tokens[1][ct],'9');
		if (http_tokens[1][ct]>'9' || http_tokens[1][ct]<'0'){
			fprintf(stderr, "A decimal number is expected %s\n",
					http_tokens[1]);
			return -1;
		}

}

	if (!strcmp(line_tokens[1], "..") || !strcmp(line_tokens[1], "//")) {
		fprintf(stderr, "Security violation for the file name: %s\n",
				line_tokens[1]);
		return -1;
	}

	// return the file name upon correct GET request
	strcpy(filename, line_tokens[1]);
	//strcat(filename,"\0");
//	fprintf(stderr, "======>Return the filename: %s with len=%d\n"	, filename,strlen(filename));

	return 0;

}

/**********************************************
 * return_result
 - returns the contents of a file to the requesting client
 - parameters:
 - fd is the file descriptor obtained by accept_connection()
 to where you wish to return the result of a request
 - content_type is a pointer to a string that indicates the
 type of content being returned. possible types include
 "text/html", "text/plain", "image/gif", "image/jpeg" cor-
 responding to .html, .txt, .gif, .jpg files.
 - buf is a pointer to a memory location where the requested
 file has been read into memory (the heap). return_result
 will use this memory location to return the result to the
 user. (remember to use -D_REENTRANT for CFLAGS.) you may
 safely deallocate the memory after the call to
 return_result (if it will not be cached).
 - numbytes is the number of bytes the file takes up in buf
 - returns 0 on success, nonzero on failure.
 ************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
// send headers back to the socketfd, connection: close, content-type, content-length, etc
// then finally send back the resulting file
// then close the connection

	fprintf(stderr, "Return result to client, %d bytes\n", numbytes);

	FILE *stream = fdopen(fd, "w");
	if (stream == NULL) {
		fprintf(stderr, "Unable to open fd %d. %s", fd, strerror(errno));
		return -1;
	}
	fprintf(stream,
			"HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\nConnection: Close\n\n",
			content_type, numbytes);
	fwrite((void*) buf, 1, numbytes, stream);
	fclose(stream);

	return 0;
}

/**********************************************
 * return_error
 - returns an error message in response to a bad request
 - parameters:
 - fd is the file descriptor obtained by accept_connection()
 to where you wish to return the error
 - buf is a pointer to the location of the error text
 - returns 0 on success, nonzero on failure.
 ************************************************/
int return_error(int fd, char *buf) {
// send 404 headers back to the socketfd, connection: close, content-type: text/plain, content-length
// send back the error message as a piece of text.
// then close the connection

//	char httpmsg[MAX_HTTP_OUTGOING_HEADER_SIZE];
//	int nheader_byte =
//			sprintf(httpmsg,
//					"HTTP/1.0 404 Not Found\nContent-type: text/plain\nContent-Length: %d\nConnection: Close\n\nError-message: %s\n",
//					-1, buf);
//	fprintf(stderr,"%s",httpmsg);
//	fprintf(stderr, "Return error to client, %d bytes\n", nheader_byte);
	FILE *stream = fdopen(fd, "w");
	if (stream == NULL) {
		fprintf(stderr, "Unable to open fd %d. %s", fd, strerror(errno));
		return -1;
	}
	fprintf(stream,
			"HTTP/1.0 404 Not Found\nContent-type: text/plain\nContent-Length: %d\nConnection: Close\n\nError-message: %s\n",
			-1, buf);
//	fprintf(stream, "%s", httpmsg);
	fclose(stream);
	/* A 404 error might look like this string, where %d is the content length:
	 * "HTTP/1.0 404 Not Found\nContent-Length: %d\nConnection: Close\n\n"
	 */

	return 0;

}

// ******************* Helper functions ***********************
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
	snew = s + strspn(s, delimiters);
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok(t, delimiters) != NULL)
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++)
			;

	if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1;
	}

	if (numtokens == 0)
		free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok(NULL, delimiters);
	}
	*((*argvp) + numtokens) = NULL;
	return numtokens;
}

void freemakeargv(char **argv) {
	if (argv == NULL)
		return;
	if (*argv != NULL)
		free(*argv);
	free(argv);
}

