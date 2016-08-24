#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#define MAX_COMMAND_LENGTH 1000
#define whitespace " "
#define newline "\n"

/* Stores all the commands in input line
 * as a doubly linked list.
 */
typedef struct node {
	struct node *prev;
	struct node *next;
	char literal[100];
}node;

/* Initialize the node to default values. */
void init_node(node *s)
{
	s->prev = NULL;
	s->next = NULL;
	memset(s->literal, '\0', 100);
}

node *scan_input(char *input)
{
	node *start = (node *)malloc(sizeof(node));
	init_node(start);
	node *iterator = start;
	char buffer[100];
	memset(buffer, '\0', 100);

	while (1) {
		if ((*input == ' ' || *input == '\0') && strlen(buffer) != 0) {
			strncpy(iterator->literal, buffer, strlen(buffer));
			memset(buffer, '\0', strlen(buffer));
			node *new = (node *)malloc(sizeof(node));
			init_node(new);
			iterator->next = new;
			new->prev = iterator;
			iterator = new;
		}
		else if (*input != ' ')
			strncat(buffer, input, 1);
		
		if (*input == '\0')
			break;
		input++;
	}
	
	if (strlen(start->literal) == 0) {
		return NULL;
	}
	else {
		iterator->prev->next = NULL;
		free(iterator);
	}
	return start;
}

void free_list(node *start)
{
	node *i = start;
	while(start != NULL) {
		i = start->next;
		free(start);
		start = i;
	}
}

void save_in_history(node *start)
{
	/*Task to do
	 * 3. !! - Run last command.
	 * 4. !n - !(followed by number) : Run this command.
	 */
	FILE *fp = fopen("history.dat", "a+");
	char chr;
	chr = getc(fp);
	/* Count number of lines. */
	int lines = 1;
	while(chr!=EOF)
	{
		if(chr == '\n')
			lines += 1;
		chr = getc(fp);
	}
	node* temp = start;
	char buf[MAX_COMMAND_LENGTH];
	memset(buf, '\0', strlen(buf));
	while(temp!=NULL)
	{
		strncat(buf, temp->literal, strlen(temp->literal));
		strncat(buf, whitespace, 1);
		temp = temp->next;
	}
	strncat(buf, newline, 1);
	fprintf(fp, "%d %s", lines, buf);
	fclose(fp);
}

char **get_cmd(node *start)
{
	int len = 0;
	node *i = start;
	while (i != NULL) {
		i = i->next;
		len++;
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

void run_child(node *start)
{
	char **cmd = get_cmd(start);
	pid_t pid;
	/* Child process creation. */
	pid = fork();
	if (pid == 0) {
		/* Child process. */
		execvp(*cmd, cmd);
		printf("Child failed\n");
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

void termination_handler(int signum)
{
	printf("\n[MJ] ");
	fflush(stdout);
}

int main(void)
{
	char c = '\0', input[1024];
	memset(input, '\0', 1024);
	printf("[MJ] ");

	/* If user press Ctrl+C then stop the processes. */
	signal(SIGINT, termination_handler);

	while(c != EOF) {
		c = getchar();
		node *start;
		if (c == '\n') {
			if (input[0] != '\0') {
				/* Start points to a linked list
				 * which contains all the tokens
				 * separated by spaces.
				 */
				start = scan_input(input);
				run_child(start);
				/* Saves the recent command in history. */
				save_in_history(start);

				/* Input buffer should be empty
				 * and linked list should be free.
				 */
				free_list(start);
				memset(input, '\0', 1024);
			}
			printf("[MJ] ");
		}
		else {
			strncat(input, &c, 1);
		}
	}
	printf("\n");
	return 0;
}
