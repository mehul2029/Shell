#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "cmd.h"

/* Convert the linked list into a array of strings - cmd */
char **get_cmd(node *start)
{
	node* temp = start;
	int len = 0;
	node *i = start;
	while (i != NULL) {
		i = i->next;
		len++;
		/* We will break the first command from nested piped input. */
		if ((i != NULL) && !strcmp(i->literal, "|"))
			break;
	}
	char **cmd = (char **)malloc(sizeof(char *)*(len + 1));
	cmd[len] = NULL;
	i = start;
	int j = 0;
	while (j < len) {
		cmd[j] = (char *)malloc(sizeof(strlen(i->literal)));
		strcpy(cmd[j], i->literal);
		j++;
		i = i->next;
	}
	return cmd;
}

/* Free memory alloted for the cmd */
void free_cmd (char **cmd)
{
	while(*cmd != NULL) {
		free(*cmd);
		cmd++;
	}
}

/* Run a child process for a non-piped command. */
void run_cmd(node *start)
{
	char **cmd = get_cmd(start);
	pid_t pid;
	pid_t ppid = getpid();
	/* Check if command is `history` or either of the bang commands.
	 * If yes, run our custom implementations.
	 */
	if(!strcmp(start->literal, "history")) {
		display_history();
	}
	else if(!strcmp(start->literal, "!!")) {
		run_last_cmd();
	}
	else if(start->literal[0]=='!') {
		run_nth_cmd(start->literal);
	}
	else {
		/* Child process creation. */
		pid = fork();
		if (pid == 0) {
			/* Child process. */
			execvp(*cmd, cmd);

			printf("%s: command not found\n", cmd[0]);
			/* Invalid command terminates the child. */
			exit(1);
		}
		else if (pid < 0) {
			/* Child not delivered. */
			perror("Slave refused");
			exit(1);
		}
		else /* Parent process. */
			wait(NULL);
	}
	free_cmd(cmd);
}

int is_pipe(node *start)
{
	node *a = start;
	int pipe = 0;
	while(start != NULL) {
		if (!strcmp(start->literal, "|")) {
			if((start->prev == NULL) || (start->next == NULL))
				return -1;
			pipe = 1;
		}
		start = start->next;
	}
	return pipe;
}

/* Run the piped commands recursively. */
void run_pipe(node *start, int pipe_input)
{
	int std_output = dup(STDOUT_FILENO);	// Save stdout for later use.
	int fd[2];								// File descriptor for piping.

	/* Forming a pipe. */
	if(pipe(fd) < 0) {
		perror("Pipe failure.\n");
		return;
	}
	/* Termination of recursive process. */
	if(start == NULL)
		return;

	char **cmd = get_cmd(start);
	
	/* `start` points to next command of input*/
	node *i = start;
	while (strcmp(i->literal, "|") && (i->next != NULL))
		i = i->next;
	start = i->next;

	pid_t ppid = getpid(); 			// Get parent process id
	pid_t pid = fork(); 			// Make a child process

	if (pid < 0)
		perror("Child failed to born");
	else if (pid == 0) {
		/* Child process. */

		close(fd[0]);
		dup2(pipe_input, 0);		// Change stdin to pipe.

		/* Close fd[1], when it's the last
		 * command to be excecuted.
		 */
		if(start == NULL)
			close(fd[1]);
		else {
			close(std_output);
			dup2(fd[1], 1);		// Change stdout to pipe.
		}		
		
		execvp(*cmd, cmd);		// Execute the commad.
		
		/* In case command is invalid. */
		printf("%s: unknown command\n", cmd[0]);
		fflush(stdout);

		/* should never be reached. */
		exit(1);
	}
	else {
		/* Parent Process. */
		wait(NULL);				// Waiting for child to stop.
		close(fd[1]);
		run_pipe(start, fd[0]);
		close(fd[0]);
		free_cmd(cmd);
	}
}

/* Calls run_pipe to exceute piped commands recursively. */
void run_pipe_cmd(node *start)
{
	int pipe_input = dup(STDIN_FILENO);	
	run_pipe(start, pipe_input);
	close(pipe_input);
}