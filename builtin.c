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

		if (child_error == -1000) {
			/* If user goes in or above "/home/" directory. */
			printf("AADM: Access denied\n");
		}
		else if (child_error) {
			/* In case chdir fails. */
			printf("AADM: %s: no such file or directory.\n", path);
		}
		else
			cd_helper(path, 1);
	}
}

int cd_helper(char *path, int print)
{
	int error = 0;
	char abs_path[MAX_PATH_LENGTH];
	/* Check if user wants to switch to home directory. */
	if(path[0]=='_') {
		const char *homedir;
		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}
		error = chdir(homedir);
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

		if (print)
			printf("%s\n", abs_path);
	}

	/* Adiya, do this:
	 * pwd should have a sub-string "/home/user" here user can be mj or so.
	 * program terminate on "/"
	 * if not, then set error = -1000
	*/
	return error;
}