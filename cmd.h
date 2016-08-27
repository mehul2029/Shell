#ifndef __CMD_H_INCLUDED__
#define __CMD_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "history.h"
#include "linkedlist.h"

char **get_cmd(node *);
void free_cmd (char **);
int is_pipe(node *);
void run_cmd(node *);
void run_pipe(node *, int);
void run_pipe_cmd(node *);

#endif
