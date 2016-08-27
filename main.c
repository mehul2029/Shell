#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "history.h"
#include "linkedlist.h"
#include "cmd.h"

/* Runs when Ctrl+c is pressed. */
void termination_handler(int signum)
{
	printf("\n>>  ");
	fflush(stdout);
}

int main(void)
{
	char c = '\0', input[1024];
	memset(input, '\0', 1024);
	printf(">> ");

	/* If user press Ctrl+C then stop the processes. */
	signal(SIGINT, termination_handler);

	while(1) {
		c = getchar();
		node *start;
		if (c == '\n') {
			if (input[0] != '\0') {
				/* Start points to a linked list
				 * which contains all the tokens
				 * separated by spaces.
				 */
				start = scan_input(input);

				int pipe = is_pipe(start);

				/* Exit the shell. */
				if (!strcmp(start->literal, "exit"))
					exit(0);

				if (pipe == 1)
					run_pipe_cmd(start);
				else if (pipe == 0)
					run_cmd(start);
				else
					printf("MJ: syntax error near unexpected token '|'\n");
				/* Saves the recent command in history. */
				save_in_history(start);

				/* Input buffer should be empty
				 * and linked list should be free.
				 */
				free_list(start);
				memset(input, '\0', 1024);
			}
			printf(">> ");
			// printf(" :: %d\n", c);
		}
		else {
			strncat(input, &c, 1);
		}
	}
	printf("\n");
	return 0;
}
