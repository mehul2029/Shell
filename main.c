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
void termination_handler(int);
int is_pipe(node *);
void run_cmd(node *);
void run_pipe(node *, int);
void run_pipe_cmd(node *);

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
