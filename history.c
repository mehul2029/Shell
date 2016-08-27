#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "history.h"

/* Display history of commands exceuted. */
void display_history()
{
	FILE* fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen("history.dat", "r");
	if (fp == NULL)
		exit(1);
	while ((read = getline(&line, &len, fp)) != -1) {
		printf("%s", line);
	}
	fclose(fp);
	if(line)
		free(line);
}

/*!! - runs the last command given user. */
void run_last_cmd()
{
	FILE* fp;
	char *line1 = NULL, *line2 = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen("history.dat", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	while ((read = getline(&line1, &len, fp)) != -1) {
		line2 = line1;
	}
	node *start = (node *)malloc(sizeof(node));
	line2[strlen(line2)-1] = '\0';
	start = scan_input(line2);
	start = start->next;
	free(start->prev);
	start->prev = NULL;
	if(is_pipe(start)==1)
		run_pipe_cmd(start);
	else if(is_pipe(start)==0)
		run_cmd(start);
	else
		printf("AADM: syntax error near unexpected token '|'\n");
}

/* When user wants to run a specific command
 * via looking at history. !n - runs the nth
 * indexed command.
 */
void run_nth_cmd(char *buf)
{
	FILE* fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen("history.dat", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	int no_of_lines = 1;
	/* Get the line number from buf. */
	int buf_length = strlen(buf);
	int i = 0, n = 0;
	while(buf_length--) {
		char ch = buf[i];
		if(ch>='0' && ch<='9')
			n = n*10 + ch - '0';
		i++;
	}
	/* Read the nth command from history for execution. */
	while (((read = getline(&line, &len, fp)) != -1) && no_of_lines < n) {
		no_of_lines++;
	}
	node* start = (node*)malloc(sizeof(node));
	line[strlen(line)-1] = '\0';
	start = scan_input(line);
	start = start->next;
	free(start->prev);
	start->prev = NULL;
	
	/* Differentiating piped and non-piped commands. */
	if(is_pipe(start)==1)
		run_pipe_cmd(start);
	else if(is_pipe(start)==0)
		run_cmd(start);
	else
		printf("MJ: syntax error near unexpected token '|'\n");
}



/* Saves the latest command given by user in history. */
void save_in_history(node *start)
{
	/* Do not store any of the following commands. */
	if(!strcmp(start->literal,"!!") || start->literal[0]=='!') {
		return;
	}
	FILE *fp = fopen("history.dat", "a+");
	char chr;
	chr = getc(fp);
	/* Count number of lines. */
	int no_of_lines = 1;
	while(chr!=EOF) {
		if(chr == '\n')
			no_of_lines += 1;
		chr = getc(fp);
	}
	node* temp = start;
	char buf[MAX_COMMAND_LENGTH];
	memset(buf, '\0', strlen(buf));
	while(temp!=NULL) {
		strncat(buf, temp->literal, strlen(temp->literal));
		strncat(buf, whitespace, strlen(whitespace));
		temp = temp->next;
	}
	strncat(buf, newline, 1);
	fprintf(fp, "%d %s", no_of_lines, buf);
	fclose(fp);
}