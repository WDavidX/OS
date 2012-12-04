#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"

#define MAX_THREADS 100
#define MAX_QLEN 100
#define MAX_CE 100

// structs:
typedef struct request_queue {
   int fd;
   char *request;
} request_queue_t;

// globals:
FILE *logfile;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t req_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t req_queue_notfull = PTHREAD_COND_INITIALIZER; // wait until queue isn't full (dispatch)
pthread_cond_t req_queue_notempty = PTHREAD_COND_INITIALIZER; // wait until queue isn't empty (worker)
request_queue_t requests[MAX_QLEN];
int req_remove_index = 0;
int req_insert_index = 0;
int req_num = 0;

int global_timestamp = 0;

int qlen;

void * dispatch(void *arg) {

   int id = (int)arg;
   int fd;
   char mybuf[1024];
   char *reqptr;

   // printf("Dispatch %d started\n",id);

   while (1) {

      // printf("Thread %d waiting for request...\n",id);
      fd = accept_connection();

      if (fd < 0) {
         return NULL;
      }

      if (get_request(fd,mybuf) != 0) {
         // printf("Bad request, thread id=%d\n",id);
         continue; // back to top of the loop!
      }
      // printf("Dispatch %d got request: %s\n",id,mybuf);
      reqptr = (char *)malloc(strlen(mybuf)+1);
      strcpy(reqptr,mybuf);

      // put request in queue
      pthread_mutex_lock(&req_queue_mutex);
      while (req_num == qlen) // the queue is full!
         pthread_cond_wait(&req_queue_notfull,&req_queue_mutex);

      requests[req_insert_index].fd = fd;
      requests[req_insert_index].request = reqptr;

      req_num++;
      req_insert_index = (req_insert_index + 1) % qlen; // ring buffer

      // printf("disp[%d] inserted req into queue: %s\n",id,reqptr);

      pthread_cond_signal(&req_queue_notempty);
      pthread_mutex_unlock(&req_queue_mutex);

   }
   return NULL;
}

void * workerBad(void *arg) {

   int id = (int)arg;
   int fd, i, found, filesize, myreqnum;
   void * memory;
   char *hit_or_miss;
   char mybuf[1024];
   myreqnum = 0;

   while (1) {

      // get request from queue
      pthread_mutex_lock(&req_queue_mutex);
      while (req_num == 0) // the queue is empty!
         pthread_cond_wait(&req_queue_notempty,&req_queue_mutex);

      fd = requests[req_remove_index].fd;
      if (fd == -9999) { // special meaning to this 'request': we must exit!!!
         pthread_mutex_unlock(&req_queue_mutex); // dont forget to unlock first!
         return NULL;
      }

      strcpy(mybuf,requests[req_remove_index].request);
      free(requests[req_remove_index].request);

      req_num--;
      req_remove_index = (req_remove_index + 1) % qlen; // ring buffer

      pthread_cond_signal(&req_queue_notfull);
      pthread_mutex_unlock(&req_queue_mutex);
      myreqnum++;

      if (strcmp(mybuf,"/")==0)
         strcpy(mybuf,"/index.html");

   }
   return NULL;
}

void * worker(void *arg) {

   int id = (int)arg;
   int fd, i, filesize, myreqnum;
   void * memory;
   char mybuf[1024];
   myreqnum = 0;

   while (1) {

      // get request from queue
      pthread_mutex_lock(&req_queue_mutex);
      while (req_num == 0) // the queue is empty!
         pthread_cond_wait(&req_queue_notempty,&req_queue_mutex);

      fd = requests[req_remove_index].fd;
      if (fd == -9999) { // special meaning to this 'request': we must exit!!!
         pthread_mutex_unlock(&req_queue_mutex); // dont forget to unlock first!
         return NULL;
      }

      strcpy(mybuf,requests[req_remove_index].request);
      free(requests[req_remove_index].request);

      req_num--;
      req_remove_index = (req_remove_index + 1) % qlen; // ring buffer

      pthread_cond_signal(&req_queue_notfull);
      pthread_mutex_unlock(&req_queue_mutex);
      myreqnum++;

      if (strcmp(mybuf,"/")==0)
         strcpy(mybuf,"/index.html");

      // increment global timestamp
      int filefd;
      if ((filefd = open(mybuf+1,O_RDONLY)) == -1) {
         return_error(fd,"File not found.");
         pthread_mutex_lock(&log_mutex);
         fprintf(logfile,"[%d][%d][%d][%s][%s]\n",id,myreqnum,fd,mybuf,"File not found.");
         fflush(logfile);
         pthread_mutex_unlock(&log_mutex);
         continue; // next request => top of while loop
      }
// do we even need this part?
         struct stat st;
         fstat(filefd,&st);
         filesize = st.st_size;
         memory = malloc(filesize);
         read(filefd,memory,filesize);
         close(filefd);
    

      pthread_mutex_lock(&log_mutex);
      fprintf(logfile,"[%d][%d][%d][%s][%d]\n",id,myreqnum,fd,mybuf,filesize);
      fflush(logfile);
      pthread_mutex_unlock(&log_mutex);

      int reqlen = strlen(mybuf);
      char * contenttype;
      if (strcmp(mybuf+reqlen-5,".html")==0) {
         contenttype = "text/html";
      } else if (strcmp(mybuf+reqlen-4,".jpg")==0) {
         contenttype = "image/jpeg";
      } else if (strcmp(mybuf+reqlen-4,".gif")==0) {
         contenttype = "image/gif";
      } else {
         contenttype = "text/plain";
      }

      if (return_result(fd,contenttype,memory,filesize) != 0) {
         printf("Couldn't return result, thread id=%d\n",id);
      }

	free(memory);
   }
   return NULL;
}

int main(int argc, char **argv) {

   pthread_t dispatch_threads[MAX_THREADS];
   pthread_t worker_threads[MAX_THREADS];

   logfile = fopen("webserver_log","w");

   if (argc != 6) {
      printf("usage: %s port path num_dispatch num_workers qlen\n",argv[0]);
      return -1;
   }

   if (chdir(argv[2])!=0) {
      perror("couldn't change directory to server root");
      return -1;
   }

   int port = atoi(argv[1]);
   int num_dispatch = atoi(argv[3]);
   int num_worker = atoi(argv[4]);
   qlen = atoi(argv[5]);

   init(port);

   if (num_dispatch > MAX_THREADS || num_dispatch < 1) {
      fprintf(stderr,"Invalid number of dispatch threads.\n");
      return -2;
   }
   if (num_worker > MAX_THREADS || num_worker < 1) {
      fprintf(stderr,"Invalid number of worker threads.\n");
      return -2;
   }
   if (qlen > MAX_QLEN || qlen < 1) {
      fprintf(stderr,"Invalid qlen size.\n");
      return -2;
   }
    printf("Starting server on port %d: %d disp, %d work\n",port,num_dispatch,num_worker);

   struct timeval time_start, time_end;
   int j, i;

   for (i=0; i < num_worker; i++) {
      pthread_create(worker_threads+i,NULL,worker,(void *)i);
   }
   for (i=0; i < num_dispatch; i++) {
      pthread_create(dispatch_threads+i,NULL,dispatch,(void *)i);
   }

   for (i=0; i < num_dispatch; i++) {
      pthread_join(dispatch_threads[i],NULL);
   }
   // at this point, all the dispatch threads have managed to exit.
   // the request queue could be empty, non-empty, and/or the workers
   //    could be busy doing stuff right now.
   // => insert a 'dummy' item into the request queue, so that when workers
   //    receive it, it means that they should exit.
   pthread_mutex_lock(&req_queue_mutex);
   while (req_num == qlen) // the queue is full!
      pthread_cond_wait(&req_queue_notfull,&req_queue_mutex);
   requests[req_insert_index].fd = -9999;
   requests[req_insert_index].request = NULL;
   req_num++;
   req_insert_index = (req_insert_index + 1) % qlen; // ring buffer
   pthread_cond_broadcast(&req_queue_notempty); // wake EVERYONE UP!!
   pthread_mutex_unlock(&req_queue_mutex);

   // the workers should be able to exit now.
   for (i=0; i < num_worker; i++) {
      pthread_join(worker_threads[i],NULL);
   }

   fclose(logfile);
   printf("all threads have exited, main thread exiting...\n");
    
   return 0;

}

