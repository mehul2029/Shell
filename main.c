#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* Stores all the commands in input line
 * as a doubly linked list.
 */
typedef struct node {
	struct node *prev;
	struct node *next;
	char literal[100];
}node;

/* Initialize the node to default values
 */
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

	/* If user press Ctrl+C then stop the processes */
	signal(SIGINT, termination_handler);

	while(c != EOF) {
		c = getchar();
		node *start;
		if (c == '\n') {
			printf("[MJ] ");
			if (input[0] != '\0') {
				/* start points to a linked list
				 * which contains all the tokens
				 * seperated by spaces
				 */
				start = scan_input(input);
			}
			/* input buffer should be empty 
			 * and linked list should be free
			 */
			free_list(start);
			memset(input, '\0', 100);
		}
		else {
			strncat(input, &c, 1);
		}
	}
	printf("\n");
	return 0;
}
