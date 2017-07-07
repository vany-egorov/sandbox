#ifndef __VA_LOG_STD__
#define __VA_LOG_STD__


typedef enum log_level_enum {
	/* For pervasive information on states of all elementary constructs.
	 * Use 'Trace' for in-depth debugging to find problem parts of a function,
	 * to check values of temporary variables, etc.
	 */
	LOG_LEVEL_TRACE,

	/* For detailed system behavior reports and diagnostic messages
	 * to help to locate problems during development.
	 */
	LOG_LEVEL_DEBUG,

	/* For general information on the application's work.
	 * Use 'Info' level in your code so that you could leave it
	 * 'enabled' even in production. So it is a 'production log level'.
	 */
	LOG_LEVEL_INFO,

	/* For indicating small errors, strange situations,
	 * failures that are automatically handled in a safe manner.
	 */
	LOG_LEVEL_WARN,

	/* For severe failures that affects application's workflow,
	 * not fatal, however (without forcing app shutdown).
	 */
	LOG_LEVEL_ERROR,

	/* For producing final messages before application’s death. */
	LOG_LEVEL_CRITICAL,

	/* A special log level used to turn off logging. */
	LOG_LEVEL_OFF,
} LogLevel;


LogLevel log_level_parse(char* s);


#endif /* __VA_LOG_STD__ */
