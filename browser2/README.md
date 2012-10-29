* PURPOSE
The lab 2 aims to deisgn a multi-process broswer program. It has a master controller tab in which the related management operatiions are performed, incuding creating a new tab, entering new URL address and specifiyng the website to be rendered in a given tab.

* Compiling
Before compiling the the source code, please make sure the related libraries are installed on the host. For example, for Ubuntu, please install the libgtc, libwebkit and libwebkitgit libraries using apt-get. The versions may vary somehow from systems to systems.

To compile the source code, please enter

$ make

Note: we modified the Makefile so that the wrapper.o can be compiled seperately, avoiding duplicated compiling everytime.

* Running the extutable
To run the excutable, please go the corresponding directory and type

$ ./brower

* What exactly the program does
The program has processes in three catagories: the rounter process, the controller tab process and some URL rendering tab processes. The latter two open window instances.

Firstly, the router process forks a process with controller tab open after startup. Then it forks new process for URL rendering tab later during function calls. The router process polls all requests from its child and sends requests to child processes. All IPC in this project will involve the rounter process on one side.

The controller tab proces takes input from the user. When the create new tab button is clicked, it sends a CREATE_TAB request to the router process. When the URL is entered with a tab index, the it sends a NEW_URI_ENTERED to the rounter process. When its close button is clicked, it sends a TAB_KILLED requst to the rounter.

The URL rendering tab processes listen the requests from the router process. When the request is a TAB_KILLED request, it kills itself. When the request is a NEW_URI_ENTERED request, it renders a new URL, which is specified in the request, in itself.

* Assumptions
1 The max number of tabs will be defined in UNRECLAIMED_TAB_COUNTER=10
2 The newly created tab will have tab_indices in ascending order
3 Once a tab is closed, it cannot be reused or recreated to render a website later.

* Stratagies for error handling
** Handling invalid tab index specification in CONTROLLER tab
The tab index will be firstly examed in function uri_entered_cb: 	if(tab_index <= 0 || tab_index > UNRECLAIMED_TAB_COUNTER){} . This check is performed immediately after the user hits the enter key. It will print an error message and return immediately if the tab index is invalid. Thus, the subsequence operations will be omitted. Specifically, no reqeust will be generated and sent to the router process.


** Closing the CONTROLLER tab
When the close button is clicked, the CONTROLLER tab process generates a series of requests from total_tabs-1 downto 0. Those reqeuests will be sent to the router which will send the killing command to all its children. All requests will be blocked. Therefore, an URL tab will be closed only after the previous killing request is successfully received by the rounter.

** Error code returned by various system/wrapper-library calls
//TODO

** Broken pipe
There will be no broken pipe message printed. If the user tries to open a link in an invalid tab number, a message will be printed to the command line "Could not read from pipe: Bad fild descriptor".