#include <stdio.h>     /* printf */
#include <stddef.h>    /* NULL */
#include <errno.h>     /* errno */
#include <signal.h>    /* SIGINT, SIGTERM */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */
#include <semaphore.h> /* sem_t, sem_init, sem_wait, sem_post, sem_destroy */

#include "./va/va.h"


sem_t sem = { 0 };


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

static int signal_init(void) {
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

static int signal_wait(void) {
	sem_wait(&sem);
	return 0;
}

static int va_parser_parse_cb(void *ctx, void *atom, VAAtomKind atom_kind, uint64_t offset) {
	if (atom_kind == VA_ATOM_KIND_MPEGTS_HEADER)
		return 0;

	printf("0x%08llX | %p | %p | %d\n", offset, ctx, atom, atom_kind);
	return 0;
}

int main (int argc, char *argv[]) {
	int ret = EX_OK;
	VAParser va_parser_s = { 0 },
	        *va_parser = &va_parser_s;
	VAParserOpenArgs va_parser_open_args = {
		.i_url_raw = "udp://239.1.1.1:5500",
		.cb = va_parser_parse_cb,
		.cb_ctx = NULL,
	};

	printf("sizes:\n");
	printf("  H264: %zu\n", sizeof(H264));
	printf("  MPEGTS: %zu\n", sizeof(MPEGTS));
	printf("  MPEGTSPSI: %zu\n", sizeof(MPEGTSPSI));

	if (ret=signal_init()) goto cleanup;

	if (va_parser_open(va_parser, &va_parser_open_args)) {
		ret = EX_SOFTWARE; goto cleanup;
	}

	va_parser_go(va_parser);

	signal_wait();

cleanup:
	va_parser_close(va_parser);
	return ret;
}
