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
	char line[LINEMAX + 1];
	node* end;

	FILE* finput;
	if (((finput = fopen(file, "rw")) == 0)) {
		perror("Invalid file!");
		exit(1);
	}
	
	while(fgets(line, LINEMAX, finput) != NULL)
	{
		node* newend = malloc(sizeof(node));
		if (newend == NULL)
		{
			perror("Could not allocate memory!");
			exit(1);
		}
		if (end == NULL)
		{
			root = end = newend;
		}
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
		if (strlen(line) > 0 && line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
		strncpy(end->data, line, LINEMAX + 1);
	}
	
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
     * Appends the everything from line
     * until a newline is reached to the
     * current text buffer. If LINEMAX is
     * reached it inserts a newline and
     * continues appending line.
     * @params line the line of text to be
     * appended
     * @returns 0 if error occurs and 1 if successful
     */
/*int appendLine(char* line){
	int i = 0, j;
	node* end = root;
	while (end != NULL && end->next != NULL)
	{
		end = end->next;
	}
	
	while (line[i] != '\0')
	{
		node* newend = malloc(sizeof(node));
		if (newend == NULL)
		{
			perror("Could not allocate memory!");
			return 0;
		}
		
		if (end != NULL)
		{
			end->next = newend;
			end = newend;
		}
		else
		{
			root = end = newend;
		}

		end->data = malloc(LINEMAX + 1);
		if (end->data == NULL)
		{
			perror("Could not allocate memory!");
			return 0;
		}
		for (j=0; line[i] != '\n' && line[i] != '\0' && j < LINEMAX; i++, j++)
		{
			end->data[j] = line[i];
		}
		end->data[j+1] = '\0';
	}
	return 1;
};*/

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
	node* end = root;
	for (i = 0; end != NULL && i < row; i++)
	{
		end = end->next;
	}
	if (end == NULL) return 0;
	
	if (col > LINEMAX) return 0;
	
	int len = strlen(end->data);
	if (col == LINEMAX && len == LINEMAX-1)
	{
		node* add = malloc(sizeof(node));
		if (root == NULL)
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
		
		add->next = end->next;
		end->next = add;
		return 1;
	}
	if (col == LINEMAX || col > len) return 0;
	
	if (col == len)
	{
		end->data[col] = text;
		end->data[col + 1] = '\0';
		return 1;
	}
	
	if (len == LINEMAX)
	{
		node* add = malloc(sizeof(node));
		if (root == NULL)
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
		add->data[0] = end->data[len - 1];
		add->data[1] = '\0';
		
		add->next = end->next;
		end->next = add;
		strncpy(end->data + col + 1, end->data + col, LINEMAX+1);
		end->data[col] = text;
		end->data[LINEMAX] = '\0';
		return 1;
	}
	
	strncpy(end->data + col + 1, end->data + col, LINEMAX+1);
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
     * Delete the line index
     * @returns 0 if error otherwise returns 1
     */
int deleteLine(int index){
	if (index == 0)
	{
		node* temp = root;
		root = root->next;
		free(temp->data);
		free(temp);
		return 1;
	}
	
	if (root == NULL) return 0;
	
	node* end = root;
	int i;
	for (i = 0; end->next != NULL && i < index - 1; i++)
	{
		end = end->next;
	}
	
	if (end->next == NULL) return 0;

	if (end->next->next == NULL)
	{
		free(end->next->data);
		free(end->next);
		end->next = NULL;
	}
	else
	{
		node* temp = end->next;
		end->next = end->next->next;
		free(temp->data);
		free(temp);
	}
	return 1;
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
	
	if (col > 0 && col < LINEMAX)
	{
		strncpy(end->data + col, end->data + col + 1, LINEMAX+1);
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
