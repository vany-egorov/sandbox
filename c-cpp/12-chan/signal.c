#include "signal.h"


volatile sig_atomic_t signal_sigstop_received = 0;
static sem_t sig_sem = { 0 };


static void signal_handler(int sig) {
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		printf("SIGINT/SIGTERM => gracefully shutdown\n");
		sem_post(&sig_sem);
		signal_sigstop_received = 1;
		break;
	default:
		fprintf(stderr, "caught wrong signal: %d\n", sig);
		return;
	}
}

int signal_wait(void) {
	sem_wait(&sig_sem);
	return 0;
}

int signal_init(void) {
	struct sigaction signal_action;
	signal_action.sa_handler = &signal_handler;
	sem_init(&sig_sem, 0, 0);

	if (sigaction(SIGINT, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGINT: \"%s\"", strerror(errno));
		return 1;
	}
	if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGTERM: \"%s\"", strerror(errno));
		return 1;
	}

	return 0;
}
