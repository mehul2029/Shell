#ifndef __HISTORY_H_INCLUDED__
#define __HISTORY_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "linkedlist.h"
#include "cmd.h"

#define MAX_COMMAND_LENGTH 1000
#define whitespace " "
#define newline "\n"

void display_history();
void run_last_cmd();
void run_nth_cmd(char *);
void save_in_history(node *);


#endif