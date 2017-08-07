#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#include "./color.h"


void daemonize() {
	int fd;
	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0) {  /* parent exit */
		waitpid(pid, &status, 0);
		exit(EXIT_SUCCESS);
	}

	if (setsid() < 0)
		exit(EXIT_FAILURE);

	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)  /* child exit */
		exit(EXIT_SUCCESS);

	umask(133);

	if (chdir("/"))
		exit(EXIT_FAILURE);

	for (fd = sysconf(_SC_OPEN_MAX); fd>0; fd--)
		close(fd);
}

int daemonize_write_pidfile(char *path) {
	int ret = 0;
	FILE *f = fopen(path, "w");

	if (f == NULL) {
		COLORSTDERR("failed to create pidfile \"%s\"", path);
		return 1;
	}

	if (fprintf(f, "%d", getpid()) < 0) {
		COLORSTDERR("failed write PID to pidfile: \"%s\", pid: %d",
			path, getpid());
		fclose(f);
		return 1;
	}

	fflush(f);
	fclose(f);
	return ret;
}
