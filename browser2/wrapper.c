#include "wrapper.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// See each function for a bigger description. 

char* get_entered_uri(GtkWidget* entry);
int render_web_page_in_tab(char* uri, browser_window* b_window);
int query_tab_id_for_request(GtkWidget* entry, gpointer data);
void process_all_gtk_events();
void process_single_gtk_event();
void create_add_remove_tab_button(char* label, void (*g_callback)(void), void* cb_data);
void create_labeled_tab(void* cb_data);
void alert(gchar* msg);
void delete_window_cb(GtkWidget *window, gpointer data);
int create_browser(tab_type t_type, 
		   int tab_index,
		   void (*create_new_tab_cb)(void), 
		   void (*uri_entered_cb)(void), 
		   browser_window **b_window,
		   comm_channel comm_channel);
void show_browser();



/**********
 * Name:	get_entered_uri
 * Input:	'entry'-GtkWidget for address bar in controller-tab
 * Output:      user entered 'url'
 * Function:    returns the url entered in the address bar
 */
char* get_entered_uri(GtkWidget* entry)
{
  return((char*)gtk_entry_get_text (GTK_ENTRY (entry)));
}

/**********
 * Name:	render_web_page_in_tab
 * Input:	'uri'- char array of address
 *		'b_window'- browser_window* that holds the page belonging to a tab
 * Output:      nothing
 * Function:    fetches the page data and displays it
 */
int render_web_page_in_tab(char* uri, browser_window* b_window)
{
  // webkit_web_view_open(b_window->web_view, uri); - DEPRECATED
  webkit_web_view_load_uri(b_window->web_view, uri);
  return 0;
}

/**********
 * Name:	query_tab_id_for_request
 * Input:	'entry'- GtkWidget for address bar in controller-tab when url was entered
 *		'data'- auxiliary data sent along with the entry
 * Output:      tab index
 * Function:    returns the tab number you requested
 */
int query_tab_id_for_request(GtkWidget* entry, gpointer data)
{
  browser_window *b_window = (browser_window*)data;
  const gchar* selected_tab_index = gtk_entry_get_text(GTK_ENTRY(b_window->tab_selector));
  if(selected_tab_index != NULL)
  	return(atoi(selected_tab_index));
  return -1;
}

/**********
 * Name:	process_all_gtk_events
 * Input:	none
 * Output:      none
 * Function:    Checks if any events are pending and process them. This can be used to update 
 * 		The GUI and invoke timeouts etc. while doing some time intensive computation. 
 */
void process_all_gtk_events()
{
  while(gtk_events_pending ())
    gtk_main_iteration();
}


/**********
 * Name:	process_single_gtk_event()
 * Input:	none
 * Output:      none
 * Function:    Runs a single iteration of the mainloop. If no events are 
 * 		available return (the FALSE flag). Non blocking. 
 */
void process_single_gtk_event()
{
  gtk_main_iteration_do(FALSE);
}

/**********
 * Name:	create_add_remove_tab_button
 * Input:	'label' - the string to put on the add tab button (a +)
 *		'g_callback' - the create_new_tab_cb function passed in
 *		'cb_data'- the browser_window passed in
 * Output:      none
 * Function:    Creates the 'add tab' button on the main window
 */
void create_add_remove_tab_button(char* label, void (*g_callback)(void), void* cb_data)
{
  browser_window *b_window=((browser_window*)cb_data);

  GtkWidget* new_tab_button = gtk_button_new_with_label (label);
  g_signal_connect (G_OBJECT (new_tab_button), "clicked", g_callback, cb_data);
  gtk_widget_show(new_tab_button);

  GtkWidget *window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (window, WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(window);

  gtk_notebook_append_page (GTK_NOTEBOOK (b_window->notebook), window, new_tab_button);
}

/**********
 * Name:	create_labeled_tab
 * Input:	'cb_data' - the browser_window passed in
 * Output:      none
 * Function:    Build a new scrolling window, set the size, create the button
 * 		and tab area on the window, and display it all.  Used by 
 * 		create_browser function.
 */
void create_labeled_tab(void* cb_data)
{
  GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (scrolled_window, WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), 
	GTK_POLICY_AUTOMATIC, 
	GTK_POLICY_AUTOMATIC);
  gtk_widget_show(scrolled_window);

  // Create 'button-press-event' callback event data
  browser_window* b_window = (browser_window*)cb_data;

  // Create web-page rendering area
  b_window->web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET(b_window->web_view));

  webkit_web_view_open(b_window->web_view, "about:blank");

  GtkWidget* label = gtk_label_new(b_window->tab_label);
  gtk_widget_show(label);

  // Attach tab to the browser
  gtk_notebook_append_page (GTK_NOTEBOOK (b_window->notebook), scrolled_window, label);

}

/**********
 * Name:	alert
 * Input:	'msg' - the message to display to the user
 * Output:      none
 * Function:    Display a window that the user must close before doing anything about.
 */
// display popup alert box
void alert(gchar* msg) 
{ 
  GtkWidget* dialog = gtk_dialog_new_with_buttons("Message", 
		NULL,
		GTK_DIALOG_MODAL,
		GTK_STOCK_OK,
		GTK_RESPONSE_NONE,
		NULL); // create a new dialog
  GtkWidget* content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  GtkWidget* label = gtk_label_new(msg);
  g_signal_connect_swapped (dialog,
                           "response",
                            G_CALLBACK (gtk_widget_destroy),
                            dialog);
  gtk_container_add (GTK_CONTAINER (content_area), label);
  gtk_widget_show_all (dialog);
}

/**********
 * Name:	delete_window_cb
 * Input:	'window' - Gtk widget for the window
 *		'data' - pointer to window data
 * Output:      none
 * Function:    Tells the window it was killed and quits it.
 */
void delete_window_cb(GtkWidget *window, gpointer data)
{
  browser_window* b_window = (browser_window*)data;
  child_req_to_parent new_req;
  new_req.type = TAB_KILLED;
  new_req.req.killed_req.tab_index = b_window->tab_index;
  write(b_window->channel.child_to_parent_fd[1], &new_req, sizeof(new_req));
  if(b_window->tab_index == 0)
	gtk_main_quit();
}


/**********
 * Name:	create_browser
 * Input:	'type' - either controller or url rendering tab
 *		'tab_index' - which index
 *		'create_new_tab_cb' - the call back function to create a new tab
 * 		'uri_entered_cb' - the call back function to help make a persistent window when you hit enter
 * 		'b_window' - double pointer to a browser_window type to get initialized
 * 		'comm_channel' - the channel of a particular tab to chatter on; full of FD's.
 * Output:      error description
 * Function:    Sets up the location of the browser window, 
 * 		allocates space for data etc., (building it if it's a CONTROLLER_TAB 
 * 		or making it the empty boring window for tabs), labels window pieces, 
 * 		and shows the end window result.
 */
int create_browser(tab_type t_type, 
		   int tab_index,
		   void (*create_new_tab_cb)(void), 
		   void (*uri_entered_cb)(void), 
		   browser_window **b_window,
		   comm_channel comm_channel)
{
  GtkWidget *window;
  GtkWidget *table;

  gtk_init(NULL, NULL);
  if (!g_thread_supported ())
    g_thread_init (NULL);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);

  if(t_type == CONTROLLER_TAB)
    table = gtk_table_new(2,4,FALSE);
  else 
    table = gtk_table_new(2,1,FALSE);

  gtk_container_add (GTK_CONTAINER (window), table);

  gtk_table_set_row_spacing(GTK_TABLE(table), 0, 20);

  // Allocate space for browser-window to be passed to various callbacks.
  *b_window = calloc(1, sizeof(browser_window));
  (*b_window)->web_view = NULL;
  (*b_window)->notebook = gtk_notebook_new ();
  (*b_window)->tab_index = tab_index;
  sprintf((*b_window)->tab_label, "Tab %d", tab_index);
  memcpy((*b_window)->channel.parent_to_child_fd, 
	  comm_channel.parent_to_child_fd, 
	  2*sizeof(int));
  memcpy((*b_window)->channel.child_to_parent_fd,
  	  comm_channel.child_to_parent_fd,
	  2*sizeof(int));


  gtk_notebook_set_tab_pos (GTK_NOTEBOOK ((*b_window)->notebook), GTK_POS_TOP);
  gtk_table_attach_defaults (GTK_TABLE (table), (*b_window)->notebook, 0, 6, 1, 2);
  gtk_widget_show ((*b_window)->notebook);
 
  if(t_type == CONTROLLER_TAB) 
  {
    gtk_window_set_title(GTK_WINDOW(window), "CONTROLLER-TAB");
    
    GtkWidget* url_label = gtk_label_new("URL");
    gtk_table_attach_defaults(GTK_TABLE(table), url_label, 0,1,0,1);
    gtk_widget_show(url_label);

    GtkWidget* uri_entry = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), uri_entry, 1, 2, 0, 1); 
    gtk_widget_show(uri_entry);
    g_signal_connect (G_OBJECT (uri_entry), "activate", uri_entered_cb, *b_window);
    (*b_window)->uri_entry = uri_entry;

    GtkWidget* tab_label = gtk_label_new("Tab Number");
    gtk_table_attach_defaults(GTK_TABLE(table), tab_label, 2,3,0,1);
    gtk_widget_show(tab_label);
  
    GtkWidget* tab_selector = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), tab_selector, 3, 4, 0, 1);
    gtk_widget_show(tab_selector);
    //g_signal_connect (G_OBJECT (uri_entry), "activate", uri_entered_cb, *b_window);
    (*b_window)->tab_selector = tab_selector;
  }
  else
  {
    gtk_window_set_title(GTK_WINDOW(window),"URL-RENDERING TAB");
    (*b_window)->tab_selector = NULL;
    (*b_window)->uri_entry = NULL;
  }

  if(t_type == CONTROLLER_TAB)
  {
    // Create "+" and "-" buttons (for adding and deleting new tabs) 
    // in CONTROLLER tab
    create_add_remove_tab_button("+", G_CALLBACK(create_new_tab_cb), *b_window);
    //create_add_remove_tab_button("-", G_CALLBACK(delete_tab_clicked_cb), *b_window);
  }
  else
  {
     // Create rendering window for ORDINARY tab
     create_labeled_tab(*b_window);
  }
    g_signal_connect(G_OBJECT (window), "destroy", 
		   G_CALLBACK(delete_window_cb), *b_window);
  gtk_widget_show(table);
  gtk_widget_show_all(window);
  return 0;
}


/**********
 * Name:	show_browser
 * Input:	none
 * Output:      none
 * Function:    runs everything until you tell it to quit
 */
void show_browser()
{
  gtk_main();
}
