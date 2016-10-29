#ifndef __COLOR__
#define __COLOR__


#define COLOR_BRIGHT_BLACK    "\x1B[1;30m"
#define COLOR_BRIGHT_RED      "\x1B[1;31m"
#define COLOR_BRIGHT_GREEN    "\x1B[1;32m"
#define COLOR_BRIGHT_YELLOW   "\x1B[1;33m"
#define COLOR_BRIGHT_BLUE     "\x1B[1;34m"
#define COLOR_BRIGHT_MAGENTA  "\x1B[1;35m"
#define COLOR_BRIGHT_CYAN     "\x1B[1;36m"
#define COLOR_BRIGHT_WHITE    "\x1B[1;37m"

#define COLOR_DIM_BLACK   "\x1B[2;30m"
#define COLOR_DIM_RED     "\x1B[2;31m"
#define COLOR_DIM_GREEN   "\x1B[2;32m"
#define COLOR_DIM_YELLOW  "\x1B[2;33m"
#define COLOR_DIM_BLUE    "\x1B[2;34m"
#define COLOR_DIM_MAGENTA "\x1B[2;35m"
#define COLOR_DIM_CYAN    "\x1B[2;36m"
#define COLOR_DIM_WHITE   "\x1B[2;37m"

#define COLOR_RESET "\033[0m"


#define COLORSTDERR(X, ...) fprintf(stderr, COLOR_BRIGHT_RED X COLOR_RESET "\n", ##__VA_ARGS__)


#endif // __COLOR__
