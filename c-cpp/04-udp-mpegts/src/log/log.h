#ifndef __NVIDIA_API_LOG__
#define __NVIDIA_API_LOG__


#define LOG_MAX_MESSAGE_SIZE 16384
#define LOG_FILE_MODE        0644


typedef enum {
	// For pervasive information on states of all elementary constructs.
	// Use 'Trace' for in-depth debugging to find problem parts of a function,
	// to check values of temporary variables, etc.
	LOG_LEVEL_TRACE,

	// For detailed system behavior reports and diagnostic messages
	// to help to locate problems during development.
	LOG_LEVEL_DEBUG,

	// For general information on the application's work.
	// Use 'Info' level in your code so that you could leave it
	// 'enabled' even in production. So it is a 'production log level'.
	LOG_LEVEL_INFO,

	// For indicating small errors, strange situations,
	// failures that are automatically handled in a safe manner.
	LOG_LEVEL_WARN,

	// For severe failures that affects application's workflow,
	// not fatal, however (without forcing app shutdown).
	LOG_LEVEL_ERROR,

	// For producing final messages before application’s death.
	LOG_LEVEL_CRITICAL,

	// A special log level used to turn off logging.
	LOG_LEVEL_OFF,
} log_level_t;


typedef struct {
	char        name[10];

	mode_t mode;
	FILE  *file_access;
	FILE  *file_error;
	int    fd_access;
	int    fd_error;

	log_level_t log_level_min;

	int is_stderr_stdout_enabled;

	pthread_mutex_t lock;
} log_t;

log_level_t log_level_new(char* v);
log_t      *log_new(char *name, log_level_t log_level_min,
                    char *path,
                    char *filename_acess, char *filename_error,
                    int is_stderr_stdout_enabled);
int         log_rotate(log_t *it, char *path,
                       char *filename_acess, char *filename_error);
void        log_del(log_t *it);

void log_trace(log_t *it, const char* format, ...);
void log_debug(log_t *it, const char* format, ...);
void log_info(log_t *it, const char* format, ...);
void log_warn(log_t *it, const char* format, ...);
void log_error(log_t *it, const char* format, ...);
void log_critical(log_t *it, const char* format, ...);

void log_noendl_trace(log_t *it, const char* format, ...);
void log_noendl_debug(log_t *it, const char* format, ...);
void log_noendl_info(log_t *it, const char* format, ...);
void log_noendl_warn(log_t *it, const char* format, ...);
void log_noendl_error(log_t *it, const char* format, ...);
void log_noendl_critical(log_t *it, const char* format, ...);


#endif // __NVIDIA_API_LOG__
