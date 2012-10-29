* PURPOSE
The lab aims to deisgn a multi-process broswer program. It has a master controller tab in which the related management operatiions are performed, incuding creating a new tab, entering new URL address and specifiyng the website to be rendered in a given tab.

* Compiling
Before compiling the the source code, please make sure the related libraries are installed on the host. For example, for Ubuntu, please install the libgtc, libwebkit and libwebkitgit libraries. The versions may vary somehow from systems to system.

To compile the source code, please enter

$ make

Note: we modified the Makefile so that the wrapper.o can be compiled seperated, avoiding compiling it everytime.

* Running the extutable
To run the excutable, please go the corresponding directory and type

$ ./brower

* What exactly the program does
The program have three processes in three catagories: the rounter process, the controller tab process and some URL rendering tab processes. The latter two open windows instances.

Firstly, the router process forks a process with controller tab open after startup. Then it forks new process for URL rendering tab later during function calls. The router process polls all requests from its child and sends requests to child processes. All IPC in this project will involve the rounter process on one side.

The controller tab proces takes input from the user. When the create new tab button is clicked, it sends a CREATE_TAB request to the router process. When the URL is entered with a tab index, the it sends a NEW_URI_ENTERED to the rounter process. When its close button is clicked, it sends a TAB_KILLED requst to the rounter.

The URL rendering tab processes listen the requests from the router process. When the request is a TAB_KILLED request, it kills itself. When the request is a NEW_URI_ENTERED request, it renders a new URL, which is specified in the request, in itself.

* Assumptions

* Stratagies for error handling
** Handling invalid tab index specification in CONTROLLER tab
The tab index

** Closing the CONTROLLER tab

** Error code returned by various system/wrapper-library calls

** Broken pipe
