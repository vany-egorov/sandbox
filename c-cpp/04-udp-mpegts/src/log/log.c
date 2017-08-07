#include "log.h"


int log_new(Log **out) {
	int ret = 0;
	Log *it = NULL;

	it = (Log*)calloc(1, sizeof(Log));
	if (!it) return 1;

	*out = it;
	return ret;
}

int log_init(Log *it,
	           char *name, LogLevel log_level_min,
	           char *path,
	           char *filename_acess, char *filename_error,
	           int is_stderr_stdout_enabled) {
	it->mode = LOG_FILE_MODE;
	it->fd_access = 0;
	it->fd_error = 0;

	it->log_level_min = log_level_min;
	it->is_stderr_stdout_enabled = is_stderr_stdout_enabled;

	if (log_rotate(it, path, filename_acess, filename_error)) {
		COLORSTDERR("error creating log files");
		return 1;
	}

	return 0;
}

int log_rotate(Log *it, char *path,
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

int log_into_logger(Log *it, Logger *out) {
	int ret = 0;

	out->w = it;
	out->vt = &log_logger_vt;

	return ret;
}

int log_fin(Log *it) {
	int ret = 0;

	if (it == NULL) return ret;

	if (it->file_access != NULL) {
		fclose(it->file_access);
		it->file_access = NULL;
	}

	if (it->file_error != NULL) {
		fclose(it->file_error);
		it->file_error = NULL;
	}

	if (it != NULL) free(it);

	return ret;
}

int log_del(Log **out) {
	int ret = 0;
	Log *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	ret = log_fin(it);

	free(it);
	*out = NULL;

	return ret;
}

static void log_printf(Log *it, LogLevel level, const char* format, va_list args) {
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
			"[%s] [%c] %s",
			datetime,
			log_level_short(level),
			message);
		fflush(it->file_access);
	}

	if (it->file_error != NULL) {
		if ((level == LOG_LEVEL_WARN) ||
		    (level == LOG_LEVEL_ERROR) ||
		    (level == LOG_LEVEL_CRITICAL)) {
			fprintf(it->file_error,
				"[%s] [%c] %s",
				datetime,
				log_level_short(level),
				message);
			fflush(it->file_error);
		}
	}

	if (it->is_stderr_stdout_enabled) {
		printf(
			"%s[%s]%s %s[%c]%s %s%s%s",
			COLOR_BRIGHT_WHITE, datetime, COLOR_RESET,
			c1, log_level_short(level), COLOR_RESET,
			c2, message, COLOR_RESET);
	}

	pthread_mutex_unlock(&it->lock);
}

void log_log_trace(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_TRACE, format, args); va_end(args);
};
void log_log_debug(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_DEBUG, format, args); va_end(args);
};
void log_log_info(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_INFO, format, args); va_end(args);
};
void log_log_warn(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_WARN, format, args); va_end(args);
};
void log_log_error(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_ERROR, format, args); va_end(args);
};
void log_log_critical(Log *it, const char* format, ...) {
	va_list args;
	va_start(args, format); log_printf(it, LOG_LEVEL_CRITICAL, format, args); va_end(args);
};

static void trace(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_TRACE, format, args);
}

static void debug(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_DEBUG, format, args);
}

static void info(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_INFO, format, args);
}

static void warn(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_WARN, format, args);
}

static void error(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_ERROR, format, args);
}

static void critical(void *ctx, const char* format, va_list args) {
	Log *it = (Log*)it;
	log_printf(ctx, LOG_LEVEL_CRITICAL, format, args);
}


LoggerVT log_logger_vt = {
	.trace = trace,
	.debug = debug,
	.info = info,
	.warn = warn,
	.error = error,
	.critical = critical,
};
