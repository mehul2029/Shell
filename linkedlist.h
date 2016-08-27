#ifndef __LINKEDLIST_H_INCLUDED__
#define __LINKEDLIST_H_INCLUDED__

/* Stores all the commands in input line
 * as a doubly linked list.
 */
typedef struct node {
	struct node *prev;
	struct node *next;
	char literal[100];
}node;

void init_node(node *);
node *scan_input(char *);
void free_list(node *);

#endif