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
#define CMDFAILD 1000

/* Stores all the commands in input line
 * as a doubly linked list.
 */
typedef struct node {
	struct node *prev;
	struct node *next;
	char literal[100];
}node;


/* Function signatures. */
void init_node(node *);
node *scan_input(char *);
void display_history();
void run_last_cmd();
void run_nth_cmd(char *);
void free_list(node *);
void save_in_history(node *);
char **get_cmd(node *);
void free_cmd (char **);
void run_child(node *);
int is_pipe(node *);
void run_child_pipe(node *);
void termination_handler(int);
void command_faild();
int is_pipe(node *);
void run_child_pipe(node *);

/* Initialize the node to default values */
void init_node(node *s)
{
	s->prev = NULL;
	s->next = NULL;
	memset(s->literal, '\0', 100);
}

/* Convert the input string into a linked list seperated by " " and "|" */
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
		run_child_pipe(start);
	else if(is_pipe(start)==0)
		run_child(start);
	else
		printf("MJ: syntax error near unexpected token '|'\n");
}

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
	if(is_pipe(start)==1)
		run_child_pipe(start);
	else if(is_pipe(start)==0)
		run_child(start);
	else
		printf("MJ: syntax error near unexpected token '|'\n");
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

/* Convert the linked list into a array of strings - cmd */
char **get_cmd(node *start)
{
	node* temp = start;
	int len = 0;
	node *i = start;
	while (i != NULL) {
		i = i->next;
		len++;

		/* We will break first command from nested pipe input. */
		if ((i != NULL) && !strcmp(i->literal, "|")) {
			// len--;
			break;
		}
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

void run_child(node *start)
{
	char **cmd = get_cmd(start);
	pid_t pid;
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

int fd[2];
int save_stdin, save_stdout;
/* Run the piped commands recursively. */
void run_child_pipe(node *start)
{
	/* Termination of recursive process. */
	printf("HERE1\n");
	if (start == NULL)
		return;
	char **cmd = get_cmd(start);
	
	/* `start` points to next command of input*/
	node *i = start;
	while (strcmp(i->literal, "|") && (i->next != NULL))
		i = i->next;
	start = i->next;
	pid_t ppid = getpid(); 			// Get parent process id
	pid_t pid = fork(); 			// Make a child process
printf("HERE3\n");
	if (pid < 0)
		perror("Child failed to born");
	else if (pid == 0) {
		/* Child process. */
		printf("HERE4\n");
		dup2(fd[1], 1);				// write fd[1] to 1 (default stdout)
		close(fd[0]);
		execvp(*cmd, cmd);
		printf("%s: unknow command\n", cmd[0]);
		// fflush(stdout);
		kill(ppid, SIGTERM);
		
		/* should never be reached. */
		exit(1);
	}
	else {
		/* Parent Process. */
		printf("HERE6\n");
		wait(&pid);
		printf("HERE5\n");
		close(fd[1]);
		dup2(save_stdout, 1);
		dup2(save_stdin, 0);
		dup2(fd[0], 0);
		run_child_pipe(start);
		close(fd[0]);
		dup2(save_stdin, 0);
	}
}

void xyz(node *start)
{
	int save_stdout = dup(STDOUT_FILENO);	// Save stdout for later use
	int save_stdin = dup(STDIN_FILENO);		// Save stdin for later use
	
	/* Forming a pipe. */
	pipe(fd);
	pid_t pid = fork();

	if (pid == 0) {
		run_child_pipe(start);
		printf("Exiting\n");
		fflush(stdout);	
		exit(1);
	}
	else
		wait(NULL);
}

void termination_handler(int signum)
{
	printf("\n[MJ]3 ");
	fflush(stdout);
}

void command_faild()
{
	printf("Invalid command\n");
	fflush(stdout);
}
int main(void)
{
	char c = '\0', input[1024];
	memset(input, '\0', 1024);
	printf("[MJ]1 ");

	

	/* If user press Ctrl+C then stop the processes. */
	signal(SIGINT, termination_handler);

	/* Signal from child in case of invalid command. */
	signal(SIGTERM, command_faild);

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
					xyz(start);
				else if (pipe == 0)
					run_child(start);
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
			printf("[MJ]2 ");
			// printf(" :: %d\n", c);
		}
		else {
			strncat(input, &c, 1);
		}
	}
	printf("\n");
	return 0;
}
