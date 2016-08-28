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

#include "builtin.h"

#define MAX_PATH_LENGTH 1000

/* Implementation of "cd" using chdir. */
void run_cd(char *path)
{
	pid_t pid = fork();

	if (pid == 0) {
		/* Child process. */
		int error = 0;
		error = cd_helper(path, 0);
		exit(error);
	}
	else {
		/* Parent process. */
		int child_error;
		wait(&child_error);
		if(child_error==0)
			cd_helper(path, 1);
	}
}

int cd_helper(char *path, int print)
{
	int error = 0;
	char abs_path[MAX_PATH_LENGTH];
	char cwd[MAX_PATH_LENGTH];
	getcwd(cwd, sizeof(cwd));
	/* Check if user wants to switch to home directory. */
	const char *home;
	if ((home = getenv("HOME")) == NULL) {
		home = getpwuid(getuid())->pw_dir;
	}
	if(path[0]=='_') {
		error = chdir(home);
		getcwd(abs_path, sizeof(abs_path));
		if (print)
			printf("%s\n", abs_path);
	}
	else {
		/* Check if user provided path is absolute or relative. */
		if(path[0]!='/') {
			getcwd(abs_path, sizeof(abs_path));
			strcat(abs_path, "/");
			strcat(abs_path, path);
		}
		else
			strcpy(abs_path, path);
		error = chdir(abs_path);
		getcwd(abs_path, sizeof(abs_path));
		if (print)
			printf("%s\n", abs_path);
	}
	/* Check if the user tries to go below the home directory. */
	if(strstr(abs_path, home)==NULL) {
		printf("AADM: Access denied. Cannot go above user directory.\n");
		error = 1;
	}
	/* Check if the user tries to `cd` into an invalid directory. */
	else if(error!=0) {
		printf("AADM: %s: no such file or directory.\n", path);
	}
	return error;
}
