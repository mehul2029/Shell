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

/* Implementation of "cd" using chdir. */
void run_cd(char *path)
{
	char abs_path[MAX_PATH_LENGTH];
	/* Check if user wants to switch to home directory. */
	if(path[0]=='_') {
		const char *homedir;
		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}
		chdir(homedir);
		getcwd(abs_path, sizeof(abs_path));
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
		chdir(abs_path);
		getcwd(abs_path, sizeof(abs_path));
	}
	printf("%s\n", abs_path);
}