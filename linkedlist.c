#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "linkedlist.h"

/* Initialize the node to default values */
void init_node(node *s)
{
	s->prev = NULL;
	s->next = NULL;
	memset(s->literal, '\0', 100);
}

/* Convert the input string into a linked list; delimiters are " " and "|" */
node *scan_input(char *input)
{
	node *start = (node *)malloc(sizeof(node));
	init_node(start);
	node *iterator = start, *new;
	char buffer[100];
	memset(buffer, '\0', 100);

	while (1) {
		if ((*input == ' ' || *input == '\0') && strlen(buffer) != 0) {
			strncpy(iterator->literal, buffer, strlen(buffer));
			memset(buffer, '\0', strlen(buffer));
			new = (node *)malloc(sizeof(node));
			init_node(new);
			iterator->next = new;
			new->prev = iterator;
			iterator = new;
		}
		else if (*input == '|') {
			if (strlen(buffer) != 0) {
				strncpy(iterator->literal, buffer, strlen(buffer));
				memset(buffer, '\0', strlen(buffer));
				new = (node *)malloc(sizeof(node));
				init_node(new);
				iterator->next = new;
				new->prev = iterator;
				iterator = new;
			}
			iterator->literal[0] = '|';
			new = (node *)malloc(sizeof(node));
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

/* Free the memory alloted for the linked list. */
void free_list(node *start)
{
	node *i = start;
	while(start != NULL) {
		i = start->next;
		free(start);
		start = i;
	}
}
