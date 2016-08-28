#ifndef __BUILTIN_H_INCLUDED__
#define __BUILTIN_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX_PATH_LENGTH 1000

void run_cd(char *);
int cd_helper(char *, int);

#endif