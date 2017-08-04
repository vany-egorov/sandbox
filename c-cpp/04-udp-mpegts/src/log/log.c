#include <time.h>
#include <errno.h> /* errno */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>  /* va_list, vsprintf */
#include <string.h>  /* strcmp */
#include <pthread.h> /* pthread_mutex_lock, pthread_mutex_unlock */
#include <sys/stat.h>

#include "log.h"
#include "color.h"


log_t *log_new(char *name, log_level_t log_level_min,
	             char *path,
	             char *filename_acess, char *filename_error,
	             int is_stderr_stdout_enabled) {
	log_t *it = NULL;
	it = malloc(sizeof(log_t));
	if (it == NULL) return NULL;

	it->mode = LOG_FILE_MODE;
	it->fd_access = 0;
	it->fd_error = 0;

	it->log_level_min = log_level_min;
	it->is_stderr_stdout_enabled = is_stderr_stdout_enabled;

	if (log_rotate(it, path, filename_acess, filename_error)) {
		COLORSTDERR("error creating log files");
		return NULL;
	}

	return it;
}

int log_rotate(log_t *it, char *path,
	             char *filename_acess, char *filename_error) {
	int ret = 0;
	char path_access[255] = {0};
	char path_error[255] = {0};

	pthread_mutex_lock(&it->lock);

	if (it->file_access != NULL) {
		fclose(it->file_access);
		it->file_access = NULL;
	}

	if (it->file_error != NULL) {
		fclose(it->file_error);
		it->file_error = NULL;
	}

	if ((path != NULL) && (strcmp(path, "") != 0)) {
		if ((filename_acess != NULL) && (strcmp(filename_acess, "") != 0)) {
			sprintf(path_access, "%s/%s", path, filename_acess);
			it->file_access = fopen(path_access, "a");
			if (it->file_access == NULL) {
				COLORSTDERR("error creating/rotating access log file - path-access: \"%s\"",
					path_access);
				ret = 1;
				goto cleanup;
			}

			it->fd_access = fileno(it->file_access);

			errno = 0;
			if (fchmod(it->fd_access, it->mode) < 0) {
				COLORSTDERR(
					"error setting permissions to access log file - "
					"path-access: \"%s\""
					", mode: %04o"
					", errno: %d"
					", error: \"%s\"",
					path_access,
					it->mode,
					errno,
					strerror(errno)
				);
				ret = 1;
				goto cleanup;
			}
		}

		if ((filename_error != NULL) && (strcmp(filename_error, "") != 0)) {
			sprintf(path_error, "%s/%s", path, filename_error);
			it->file_error = fopen(path_error, "a");
			if (it->file_error == NULL) {
				COLORSTDERR("error creating/rotating error log file - path-error: \"%s\"",
					path_error);
				ret = 1;
				goto cleanup;
			}

			it->fd_error = fileno(it->file_error);

			errno = 0;
			if (fchmod(it->fd_error, LOG_FILE_MODE) < 0) {
				COLORSTDERR(
					"error setting permissions to error log file - "
					"path-error: \"%s\""
					", mode: %d"
					", errno: %d"
					", error: \"%s\"",
					path_error,
					LOG_FILE_MODE,
					errno,
					strerror(errno)
				);
				ret = 1;
				goto cleanup;
			}
		}
	}

	goto cleanup;

cleanup:
	pthread_mutex_unlock(&it->lock);
	return ret;
}

void log_del(log_t *it) {
	if (it == NULL) return;

	if (it->file_access != NULL) {
		fclose(it->file_access);
		it->file_access = NULL;
	}

	if (it->file_error != NULL) {
		fclose(it->file_error);
		it->file_error = NULL;
	}

	if (it != NULL) free(it);
}

static void log_printf(log_t *it, log_level_t level, int is_endl, const char* format, va_list args) {
	char *c1 = NULL,
	     *c2 = NULL;
	char message[LOG_MAX_MESSAGE_SIZE];
	char datetime[30];

	if (level < it->log_level_min) return;

	vsprintf(message, format, args);

	c1 = log_level_color1(level);
	c2 = log_level_color1(level);
	log_datetime(datetime, sizeof(datetime));

	pthread_mutex_lock(&it->lock);

	if (it->file_access != NULL) {
		fprintf(it->file_access,
			"[%s] [%c] %s%c",
			datetime,
			log_level_short(level),
			message,
			is_endl ? '\n' : ' ');
		fflush(it->file_access);
	}

	if (it->file_error != NULL) {
		if ((level == LOG_LEVEL_WARN) ||
		    (level == LOG_LEVEL_ERROR) ||
		    (level == LOG_LEVEL_CRITICAL)) {
			fprintf(it->file_error,
				"[%s] [%c] %s%c",
				datetime,
				log_level_short(level),
				message,
				is_endl ? '\n' : ' ');
			fflush(it->file_error);
		}
	}

	if (it->is_stderr_stdout_enabled) {
		printf(
			"%s[%s]%s %s[%c]%s %s%s%s%c",
			COLOR_BRIGHT_WHITE, datetime, COLOR_RESET,
			c1, log_level_short(level), COLOR_RESET,
			c2, message, COLOR_RESET,
			is_endl ? '\n' : ' ');
	}

	pthread_mutex_unlock(&it->lock);
}

void log_trace(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_TRACE, 1, format, args); va_end(args);
};
void log_debug(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_DEBUG, 1, format, args); va_end(args);
};
void log_info(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_INFO, 1, format, args); va_end(args);
};
void log_warn(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_WARN, 1, format, args); va_end(args);
};
void log_error(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_ERROR, 1, format, args); va_end(args);
};
void log_critical(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_CRITICAL, 1, format, args); va_end(args);
};

void log_noendl_trace(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_TRACE, 0, format, args); va_end(args);
};
void log_noendl_debug(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_DEBUG, 0, format, args); va_end(args);
};
void log_noendl_info(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_INFO, 0, format, args); va_end(args);
};
void log_noendl_warn(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_WARN, 0, format, args); va_end(args);
};
void log_noendl_error(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_ERROR, 0, format, args); va_end(args);
};
void log_noendl_critical(log_t *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_CRITICAL, 0, format, args); va_end(args);
};
