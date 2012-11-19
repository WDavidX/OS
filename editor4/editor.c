#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include "textbuff.h"

#define CTRL(c) ((c) & 037)

#define min(X, Y)                \
     ({ typeof (X) x_ = (X);          \
        typeof (Y) y_ = (Y);          \
        (x_ < y_) ? x_ : y_; })


// Defined constants to be used with the message data-structure
#define EDIT 0
#define SAVE 1
#define QUIT 2
#define DEL  3

// Maximum length of the message passing queue
#define QUEUEMAX 20


// Data structure for message passing queue used to communicate
// between the router and UI thread
struct message_t{
    int data;
    int row;
    int col;
    int command;
    struct message_t *next;
} typedef message;


// mutex to control access to the text buffer
pthread_mutex_t text_ = PTHREAD_MUTEX_INITIALIZER;


// The current position of the cursor in the screen
int row;
int col;

// Lines visible in the current view of textbuff
// visible on the screen
int view_min;
int view_max;

/**
 * Removes the first message from the message queue
 * and returns it.
 */
message* pop(){
}


/**
 * Inserts a message at the back of the message queue
 */
void push(message* m_){

}


/**
 * Redraws the screen with lines min_line -> max_line visible on the screen
 * places the cursor at (r_, c_) when done. If insert_ is 1 then "INPUT MODE"
 * text is displayed otherwise if 0 it isn't.
 */
int redraw(int min_line, int max_line,int r_, int c_, int insert_){
    erase();
    if(max_line - min_line != LINES-1){
        perror("HELP");
        pthread_exit(NULL);
    }
    move(0,0);

    pthread_mutex_lock(&text_);

    for(;min_line < max_line;min_line++){
        char *line;
        if(getLine(min_line,&line) == 0)
            break;
        int j;
        for(j=0;j < strlen(line);j++){
            addch(*(line+j));
        }
        addch('\n');
    }

    pthread_mutex_unlock(&text_);

    if(insert_){
        standout();
        mvaddstr(LINES-1, COLS-20, "INPUT MODE");
        standend();
    }

    move(r_,c_);
    refresh();
    return 1;
}


/**
 * Input loop of the UI Thread;
 * Loops reading in characters using getch() and placing them into the textbuffer using message queue interface
 */
void input_mode(){
    int c;
    redraw(view_min, view_max, row, col, 1);
    refresh();
    for(;;){
        c = getch();
        if(c == CTRL('D')){
            break;
        }

        //Add code here to insert c into textbuff at (row, col) using the message queue interface.


        // ------------------------------
        if(col<COLS-1){
            col++;
        }else{
            col = 0;
            row++;
        }
        if(row > LINES - 1){
            view_min++;
            view_max++;
        }
        redraw(view_min,view_max,row,col,1);
    }
    redraw(view_min,view_max,row,col,0);
}


/**
 * Main loop of the UI thread. It reads in commands as characters
 */
void loop(){
    int c;

    while(1){
        move(row,col);
        refresh();
        c = getch();

        switch(c){
            case 'h':
            case KEY_LEFT:
                if(col > 0)
                    col--;
                else
                    flash();
                break;
            case 'j':
            case KEY_DOWN:
                if(row < LINES -2)
                    row++;
                else
                    if(view_max+1<=getLineLength())
                        redraw(++view_min,++view_max,row,col,0);
                    else
                        flash();
                break;
            case 'k':
            case KEY_UP:
                if(row > 0)
                    row--;
                else
                    if(view_min-1 > -1)
                        redraw(--view_min,--view_max,row,col,0);
                    else
                        flash();
                break;
            case 'l':
            case KEY_RIGHT:
                if (col < COLS - 1)
                    col++;
                else
                    flash();
                break;
            case 'i':
            case KEY_IC:
                input_mode();
                break;
            case 'x':
                flash();

                // Add code here to delete character (row, col) from textbuf

                // ------------------------------

                redraw(view_min,view_max,row,col,0);
                break;
            case 'w':
                flash();

                // Add code here to save the textbuf file


                // ------------------------------

                break;
            case 'q':
                endwin();

                // Add code here to quit the program

                // ------------------------------
            default:
                flash();
                break;
        }

    }
}


/**
 * Function to be used to spawn the UI thread.
 */
void *start_UI(void *threadid){

    initscr();
    cbreak();
    nonl();
    noecho();
    idlok(stdscr, TRUE);
    keypad(stdscr,TRUE);

    view_min = 0;
    view_max = LINES-1;

    redraw(view_min,view_max,row,col,0);

    refresh();
    loop();
}

/**
 * Function to be used to spawn the autosave thread.
 */
void *autosave(void *threadid){

    // This function loops until told otherwise from the router thread. Each loop:

        // Open the temporary file

        // Read lines from the text buffer and save them to the temporary file

        // Close the temporary file and sleep for 5 sec.

}

int main(int argc, char **argv){

    row = 0;
    col = 0;

    // get text file from argv

    // set up necessary data structures for message passing queue and initialize textbuff

    // spawn UI thread

    // spawn auto-save thread

    // Main loop until told otherwise

        // Recieve messages from the message queue

        // If EDIT then place the edits into the text buffer

        // If SAVE then save the file additionally delete the temporary save

        // If QUIT then quit the program and tell the appropriate threads to stop

        // If DEL then delete the specified character from the text buffer

    // Clean up data structures
}