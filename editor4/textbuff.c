#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#ifndef textbuff_h_
#include "textbuff.h"
#endif
#ifndef textbuff_c_
#define textbuff_c_

node* root;
// Maximum length of a line in the textbuffer

    /**
     * Initializes the required data structures necessary
     * for the text buffer. Additionally it opens and reads
     * file into the text buffer.
     * @params file the file to be opened and read into
     * the text buffer.
     */
void init_textbuff(char* file){
	//Allocate for line length + endl
	char line[LINEMAX + 2];
	node* end = NULL;

	//Open file
	FILE* finput;
	printf("Opening file.\n"); fflush(stdout);
	if (((finput = fopen(file, "rw")) == 0)) {
		perror("Invalid file!");
		exit(1);
	}
	
	while(fgets(line, LINEMAX + 2, finput) != NULL)
	{
		node* newend = malloc(sizeof(node));
		if (newend == NULL)
		{
			perror("Could not allocate memory!");
			exit(1);
		}
		//First node
		if (end == NULL)
		{
			root = end = newend;
		}
		//Add node
		else
		{
			end->next = newend;
			end = newend;
		}
		end->data = malloc(LINEMAX + 1);
		if (end->data == NULL)
		{
			perror("Could not allocate memory!");
			exit(1);
		}
		if (strlen(line) == LINEMAX + 1 && line[LINEMAX] != '\n') ungetc(line[LINEMAX], finput);
		if (strlen(line) > 0) line[strlen(line)-1] = '\0';
		strncpy(end->data, line, LINEMAX + 1);
	}
	
	//Empty file
	if (root == NULL)
	{
		root = malloc(sizeof(node));
		if (root == NULL)
		{
			perror("Could not allocate memory!");
			exit(1);
		}
		root->data = malloc(1);
		if (root->data == NULL)
		{
			perror("Could not allocate memory!");
			exit(1);
		}
		root->data[0] = '\0';
	}
};

    /**
     * Fetches line index from the buffer
     * and places it in returnLine
     * @params index line to fetch
     * returnLine pointer which will point to
     * a copy of the desired line. Memory allocated
     * with malloc.
     * @returns 0 if error occurs or 1 if successful
     */
int getLine(int index, char** returnLine){
	int i;
	node* end = root;
	for (i = 0; end != NULL && i < index; i++)
	{
		end = end->next;
	}
	if (end == NULL) return 0;
	
	*returnLine = end->data;
	return 1;
};

    /**
     * Inserts text into row at position col.
     * If col is zero insert at the begining of the line.
     * If the new line length is more than LINEMAX
     * then split the line at line max and insert a
     * new line in the text buffer after the current line.
     * @returns 0 if error occurs or 1 if successful
     */
int insert(int row, int col, char text){
	int i;
	node *temp, *end = root;
	if (end == NULL) return 0;
	
	//Seek the node
	for (i = 0; end != NULL && i < row; i++)
	{
		temp = end;
		end = end->next;
	}
	
	//Add a new line
	if (end == NULL && i == row) {
		node* add = malloc(sizeof(node));
		if (add == NULL)
		{
			perror("Could not allocate memory!");
			return 0;
		}
		add->data = malloc(LINEMAX + 1);
		if (add->data == NULL)
		{
			perror("Could not allocate memory!");
			return 0;
		}
		add->data[0] = text;
		add->data[1] = '\0';
		
		add->next = NULL;
		temp->next = add;
	}
	//Beyond last line
	else if (end == NULL) return 0;
	
	int len = strlen(end->data);
	//Beyond LINEMAX
	if (col > LINEMAX) return 0;
	//Last character => wrap
	else if (col == LINEMAX - 1 && len == LINEMAX-1)
	{
		return insert(row + 1, 0, text);
	}
	//Beyond the end of the line
	else if (col == LINEMAX || col > len) return 0;
	//Last character on the line
	else if (col == len)
	{
		end->data[col] = text;
		end->data[col + 1] = '\0';
		return 1;
	}
	//Insert and wrap
	else if (len == LINEMAX)
	{
		char temp = end->data[len - 1];
		insert(row + 1, 0, temp);
		len--;
	}

	end->data[len + 1] = '\0';
	for (i = len; i > col; i--)
	{
		end->data[i] = end->data[i-1];
	}
	end->data[col] = text;
	return 1;
};

    /**
     * Returns the number of lines in the textbuffer
     */
int getLineLength(){
	int i;
	node* end = root;
	for (i = 0; end != NULL; i++)
	{
		end = end->next;
	}
	return i;
};

    /**
     * Delete a single characer at (row,col)
     * from the text buffer
     * @returns 0 if error otherwise returns 1
     */
int deleteCharacter(int row, int col){
	int i;
	node* end = root;
	for (i = 0; end != NULL && i < row; i++)
	{
		end = end->next;
	}
	if (end == NULL) return 0;
	
	int len = strlen(end->data);
	if (col >= 0 && col < len && col < LINEMAX)
	{
		int i;
		for (i = col; i < len; i++)
		{
			end->data[i] = end->data[i + 1];
		}
		return 1;
	}
	return 0;
};

    /**
     * Cleans up the buffer sets the buffer length
     * to -1 represinting that the buffer is invalid
     */
void deleteBuffer(){
	while (root != NULL)
	{
		node* temp = root;
		root = root->next;
		free(temp->data);
		free(temp);
	}
};
#endif
