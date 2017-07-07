static void p(log_level_t level, const char* format, va_list args) {
}


static void trace(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}

static void debug(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}

static void info(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}

static void warn(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}

static void error(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}

static void critical(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); p(it->w, format, args); va_end(args);
}


InputVT input_udp_vt = {
	.trace = trace,
	.debug = debug,
	.info = info,
	.warn = warn,
	.error = error,
	.critical = critical,
};

