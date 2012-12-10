/* CSci4061 F2012 Assignment 5
section: 3
login: schm2225
date: 12/12/12
names: Aaron Schmitz, Weichao Xu
id: 3891645, 4284387 */

Purpose:
This program acts as a web server that implements the HTTP protocol. When run from the shell, this program binds to a port and accepts request for files in a given directory from a standard browser.

Files:
README - this file; contains information on program operation
server.c - the main server file that manages threads
util.h - the header file for the utilities
util.c - the utilities for handling requests
Makefile - the file that defines how to compile the software

Shell: 
The program can be executed as ./web_server [PORT] [PATH] [NUM_DISPATCH] [NUM_WORKERS] [QLEN]
PORT is the port to bind to and listen for requests on
PATH is the webroot to deliver requested files from
NUM_DISPATCH dictates the number of available dispatch threads
NUM_WORKERS dictates the number of available worker threads
QLEN defines the maximum length of the queue

Functionality: 
Once the program is started, it will bind to the requested port and listen for requests. Each request will be accepted, processed, and responded to by sending the appropriate file in the webroot via HTTP protocol. The connection will then be closed.


