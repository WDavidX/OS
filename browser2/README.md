* PURPOSE
The lab 2 aims to design a multi-process browser program. It has a master controller tab in which the related management operations are performed, including creating a new tab, entering new URL address and specifying which tab will be used to render the URL. All websites are rendered by different processes in different windows.

* Compiling
Before compiling the source code, please make sure the related libraries are installed on the host. For example, for Ubuntu, please install the libgtc, libwebkit and libwebkitgit libraries using apt-get. The versions may vary somehow from systems to systems. A sample compiling is performed on kh4250-01.

To compile the source code, please enter

$ make

Note: we modified the Makefile so that the wrapper.o can be compiled separately, avoiding duplicated compiling everytime.

* Running the executable
To run the executable, please go to the corresponding directory and type

$ ./browser

* What exactly the program does
The program has processes in three categories: the router process, the controller tab process and some URL rendering tab processes. The latter two open window instances.

Firstly, the router process forks a process with controller tab (with tab_index=0) open after startup. Then it forks new process for URL rendering tab later during function calls. The router process polls all requests from its child and sends requests to child processes. All IPC in this project will involve the rounter process on one side.

The controller tab process takes input from the user. When create-new-tab is clicked, it sends a CREATE_TAB request to the router process. When the URL is entered with a tab index, then it sends a NEW_URI_ENTERED to the router process. When close is clicked, it sends a TAB_KILLED request to the router.

The URL rendering tab processes listen the requests from the router process. When the request is a TAB_KILLED request, it kills itself. When the request is a NEW_URI_ENTERED request, it renders a new URL, which is specified in the request, in itself.

* Assumptions
1 The max number of tabs will be defined in UNRECLAIMED_TAB_COUNTER=10.
2 The newly created tab will have tab_index in ascending order.
3 Once a tab is closed, it cannot be reused or recreated to render a website later.

* Strategies for error handling
** Handling invalid tab index specification in CONTROLLER tab
The tab index will be firstly examed in function uri_entered_cb: 	if(tab_index <= 0 || tab_index > UNRECLAIMED_TAB_COUNTER){} . This check is performed immediately after the user hits the enter key. It will print an error message and return immediately if the tab index is invalid. Thus, the subsequence operations will be omitted. Specifically, no reqeust will be generated and sent to the router process.


** Closing the CONTROLLER tab
When the close button is clicked, the CONTROLLER tab process generates a series of requests from total_tabs-1 down to 0.This order guarantees that the CONTROLLER tab is the last one to be closed. Those requests will be sent to the router which will send the killing command to all its children. All requests will be blocked. Therefore, an URL tab will be closed only after the previous killing request is successfully received by the router.

** Error code returned by various system/wrapper-library calls
Error code return by system calls are caught by the external variable errno. If the errno is EINTER, then the corresponding polling is skipped. In other cases, error messages will be printed with perror.

Most of wrapper functions are void function thus there are no return values. The return values are generally omitted. When there is an error, the following operations are skipped.


** Broken pipe
During the the phase, there is no broken pipe message printed. If the user tries to open a link in an invalid tab number, a message will be printed to the command line "Could not read from pipe: Bad file descriptor".
