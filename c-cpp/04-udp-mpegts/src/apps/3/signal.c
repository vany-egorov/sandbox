#include "./signal.h"


static sem_t sem = { 0 };


static void signal_handler(int sig) {
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		printf("SIGINT/SIGTERM => gracefully shutdown\n");
		sem_post(&sem);
		break;
	default:
		fprintf(stderr, "caught wrong signal: %d\n", sig);
		return;
	}
}

int signal_init(void) {
	struct sigaction signal_action;
	signal_action.sa_handler = &signal_handler;
	sem_init(&sem, 0, 0);

	if (sigaction(SIGINT, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGINT: \"%s\"", strerror(errno));
		return EX_OSERR;
	}
	if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGTERM: \"%s\"", strerror(errno));
		return EX_OSERR;
	}

	return 0;
}

int signal_wait(void) {
	sem_wait(&sem);
	return 0;
}
