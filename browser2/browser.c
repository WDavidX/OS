#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

void uri_entered_cb(GtkWidget* entry, gpointer data);
int wait_for_browsing_req(int fds[2], browser_window *b_window);
int wait_for_child_reqs(comm_channel* channel, int total_tabs, int actual_tab_cnt);
int create_proc_for_new_tab(comm_channel* channel, int tab_index, int actual_tab_cnt);


/*
 * Name:		uri_entered_cb
 * Input arguments:	'entry'-address bar where the url was entered
 *			'data'-auxiliary data sent along with the event
 * Output arguments:	void
 * Function:		When the user hits the enter after entering the url
 *			in the address bar, 'activate' event is generated
 *			for the Widget Entry, for which 'uri_entered_cb'
 *			callback is called. Controller-tab captures this event
 *			and sends the browsing request to the router(/parent)
 *			process.
 */


void uri_entered_cb(GtkWidget* entry, gpointer data)
{

	if(!data)
		return;
	// Create a new browser_window
	//browser_window* b_window = (browser_window*)data;
	comm_channel channel = ((browser_window*)data)->channel;
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index == -1) return;
	if(tab_index <= 0 || tab_index > UNRECLAIMED_TAB_COUNTER){
		printf("Invalid tab index for URL!\n");
		return;
	}

	// Get the URL.
	char* uri = get_entered_uri(entry);

	// Prepare 'request' packet to send to router (/parent) process.
	child_req_to_parent new_req;

	// Fill in your code here.
	child_request req;
	new_uri_req uri_req;
	// fill in 'render in tab' field
	uri_req.render_in_tab = tab_index;
	// fill in 'uri' field
	strcpy(uri_req.uri, uri);
	req.uri_req = uri_req;

	new_req.req = req;
	// enter req.type
	new_req.type = NEW_URI_ENTERED;

	// Send the request through the proper FD.
	write(channel.child_to_parent_fd[1], &new_req, sizeof(new_req));
}

/*
 * Name:		wait_for_browsing_req
 * Input Arguments:	fds[] - file descriptor on which
 *			'ordinary' child tab listen for request
 *			from 'controller' tab to render web pages.
 * Output arguments:    0-sucess
 *			-1- otherwise
 * Function:		The 'ordinary' child-tabs processes perform
 *			two tasks at tandem.
 *			1). Process GTK+ window-related events
 *			2). Listen for URL rendering requests from
 *			    'controller' tab.
 */
int wait_for_browsing_req(int fds[2], browser_window *b_window)
{

	// Close file descriptors we won't use

	// Set up for non-blocking IO


	// Set up flags for fcntl so we can change some file descriptors
	fcntl(fds[0], F_SETFL, O_NONBLOCK);


	// Continuous loop of checking requests and processing browser events
	while(1)
	{
		usleep(5000);
		// Alternatively read incoming requests from
		// 'controller-tab' and process GTK+ events
		// for the window

		// Create a new requirement, read bytes from the proper FD.
		int size;
		child_req_to_parent request;
		while (((size=read(fds[0], &request, sizeof(request))) == -1) && (errno==EINTR));
		if ((size == -1) && (errno != EAGAIN))
		{
			perror("Could not read from pipe!");
		} else if (size == sizeof(request)) {
			switch (request.type)
			{
				case TAB_KILLED:
					process_all_gtk_events();
					close(fds[0]);
					return 0;
					break;
				case NEW_URI_ENTERED:
					render_web_page_in_tab(request.req.uri_req.uri, b_window);
					break;
				case CREATE_TAB:
				default:
					printf("Unexpected command to child!");
			}
		}

		// Use this code in the following case:
		// If read received no data && errno == EAGAIN - just process any pending events and move along.

		// No browsing request from 'controller' tab
		// Hence process GTK+ window related function (non-blocking).
		process_single_gtk_event();

		// Data! Read what it is and fill in the proper request.


		// There is a browsing request from the
		// controller tab. Render the requested URL
		// on current tab.

		// Handle all request types of CREATE_TAB, NEW_URI_ENTERED,
		// and TAB_KILLED; for example, with a switch.


		// This processes any events left in the browser
		// and shuts down the window.  Use this to handle TAB_KILLED.
		/*
		case TAB_KILLED:
			process_all_gtk_events();
			return 0;
		*/



	}

	return 0;
}

/*
 * Name:		wait_for_child_reqs
 * Input Arguments:	'total_tabs': Number of tabs created so far
 * Output arguments:  	0 : Success
 *			-1: Otherwise
 * Function:		Router (/parent) process listens for request from
 *			'controller' tab and performs the required
 *			functionality based on request code.
 */
int wait_for_child_reqs(comm_channel* channel, int total_tabs, int actual_tab_cnt)
{
	// Continue listening for child requests
	
	while(1)
	{
		// Set the read FD to noblock for all tabs; remember to check for error returns!
		


		// Sleep for 0.5 sec so we don't waste CPU time
		usleep(500000);

		// Poll (read) all tabs that exist.
		// This will handle CREATE_TAB, NEW_URI_ENTERED, and TAB_KILLED.
		int i;
		for (i = 0; i < total_tabs; i++)
		{
			int size, tab_index;
			child_req_to_parent request;
			while (((size=read(channel[i].child_to_parent_fd[0], &request, sizeof(request))) == -1) && (errno == EINTR));

			if ((size == -1) && (errno != EAGAIN))
			{
				perror("Could not read from pipe!");
			} else if (size == sizeof(request)) {
				switch (request.type)
				{
					case CREATE_TAB:
						if (total_tabs + 1 > actual_tab_cnt || total_tabs + 1 < 0) {
							printf("Invalid tab index!\n");
						} else {
							tab_index = total_tabs++;
							create_proc_for_new_tab(channel, tab_index, actual_tab_cnt);
							fcntl(channel[tab_index].child_to_parent_fd[0], F_SETFL, O_NONBLOCK);
						}
						break;
					case NEW_URI_ENTERED:
						if (request.req.uri_req.render_in_tab > 0 && request.req.uri_req.render_in_tab < total_tabs) {
							write(channel[request.req.uri_req.render_in_tab].parent_to_child_fd[1], &request, sizeof(request));
						} else {
							printf("Tab index out of bounds!\n");
						}
						break;
					case TAB_KILLED:
						tab_index = request.req.killed_req.tab_index;
						if (tab_index != 0) {
							write(channel[tab_index].parent_to_child_fd[1], &request, sizeof(request));
							wait(NULL);
							close(channel[tab_index].child_to_parent_fd[0]);
							close(channel[tab_index].parent_to_child_fd[1]);
						} else {
							for (i = total_tabs - 1; i > 0; i--)
							{
								child_req_to_parent new_req;

								tab_killed_req killed_req;
								killed_req.tab_index = i;
								child_request req;
								req.killed_req = killed_req;
								new_req.req = req;
								new_req.type = TAB_KILLED;
								write(channel[i].parent_to_child_fd[1], &new_req, sizeof(new_req));
								wait(NULL);
								close(channel[i].child_to_parent_fd[1]);
								close(channel[i].parent_to_child_fd[0]);
							}
							return 0;
						}
						break;
					default:
						printf("Invalid command to router!\n");
				}
			}
			
		}
	}

	return 0;
}

/*
 * Name:		new_tab_created_cb
 * Input arguments:	'button' - whose click generated this callback
 *			'data' - auxillary data passed along for handling
 *			this event.
 * Output arguments:    void
 * Function:		This is the callback function for the 'create_new_tab'
 *			event which is generated when the user clicks the '+'
 *			button in the controller-tab. The controller-tab
 *			redirects the request to the parent (/router) process
 *			which then creates a new child process for creating
 *			and managing this new tab.
 */
// We also refer to this as "create_new_tab_cb" in Appendix C.
void new_tab_created_cb(GtkButton *button, gpointer data)
{

	printf("this is PID = %x entering new tab created \n", getpid());
	if(!data)
		return;
	comm_channel channel = ((browser_window*)data)->channel;

	// Append your code here.

	// Create new request.
	child_req_to_parent new_req;

	// Populate it with request type, CREATE_TAB, and tab index pulled from (browser_window*) data
	create_new_tab_req new_tab_req;
	new_tab_req.tab_index = ((browser_window*)data)->tab_index;
	child_request req;
	req.new_tab_req = new_tab_req;
	new_req.req = req;
	new_req.type = CREATE_TAB;

	//Send the request to the parent (/router) process through the proper FD.
	write(channel.child_to_parent_fd[1], &new_req, sizeof(new_req));
}

/*
 * Name: 		create_proc_for_new_tab
 * Input arguments: 	tab_index: Tab index for which
 *			new child process is to be created.
 * Output arguments: 	0 -success
 *			-1-otherwise
 * Function:		Creates child process for managing
 *			new tab and establishes pipes for
 *			bi-directional communication.
 */

int create_proc_for_new_tab(comm_channel* channel, int tab_index, int actual_tab_cnt)
{
	// Create bi-directional pipes (hence 2 pipes) for
	// communication between the parent and child process.
	// Remember to error check.
	if (pipe(channel[tab_index].parent_to_child_fd)) {
		perror("Couldn't open pipe");
		return -1;
	}
	if (pipe(channel[tab_index].child_to_parent_fd)) {
		perror("Couldn't open pipe");
		return -1;
	}

	// Create child process for managing the new tab; remember to check for errors!
	// The first new tab is CONTROLLER, but the rest are URL-RENDERING type.

	pid_t pid;
	if (!(pid = fork()))
	{
	// If this is the child process,

		close(channel[tab_index].parent_to_child_fd[1]);
		close(channel[tab_index].child_to_parent_fd[0]);
		//Child process should close unused pipes and launch
		// a window for either CONTROLLER or URL-RENDERING tabs.


		browser_window *b_window = NULL;

		// 'Tab 0' is the 'controller' tab. It is
		// this tab which receives browsing requests
		// from the user and redirects them to
		// appropriate child tab via the parent.
		if(tab_index == 0)
		{
			// Create the 'controller' tab
			create_browser(CONTROLLER_TAB,
				tab_index,
				G_CALLBACK(new_tab_created_cb),
				G_CALLBACK(uri_entered_cb),
				&b_window,
				channel[tab_index]);
			// Display the 'controller' tab window.
			// Loop for events
			show_browser();

		}
		else
		{
			// Create the 'ordinary' tabs.
			// These tabs render web-pages when
			// user enters url in 'controller' tabs.

			create_browser(URL_RENDERING_TAB,
				tab_index,
				NULL,
				NULL,
				&b_window,
				channel[tab_index]);

			// Wait for the browsing requests.
			// User enters the url on the 'controller' tab
			// which redirects the request to appropriate
			// child tab via the parent-tab.

			wait_for_browsing_req(channel[tab_index].parent_to_child_fd, b_window);
		}

		close(channel[tab_index].parent_to_child_fd[0]);
		close(channel[tab_index].child_to_parent_fd[1]);
		exit(0);
	}
	else  // this is parent.
	{
		// Parent Process: close proper FDs and start
		// waiting for requests if the tab index is 0.
		channel[tab_index].pid = pid;
		close(channel[tab_index].parent_to_child_fd[0]);
		close(channel[tab_index].child_to_parent_fd[1]);
		if (tab_index == 0)
		{
			wait_for_child_reqs(channel, 1, actual_tab_cnt);
		}
	}

	return 0;
}

/*  The Main function doesn't need to do much -
* create a comm_channel and create process for a
* new tab.  This first tab will become the main
* controller tab.  As always, you should modularize
* your code and separate functionality by making new
* functions responsible for such things as
* waiting for a browsing request, waiting for child requests,
* or create a process for a new tab.  We have provided
* suggestions and signatures but you are free to modify the
* given code, as long as it still works.
*/

int main()
{
	comm_channel channels[UNRECLAIMED_TAB_COUNTER];
	create_proc_for_new_tab(channels, 0, UNRECLAIMED_TAB_COUNTER);
	return 0;
}

