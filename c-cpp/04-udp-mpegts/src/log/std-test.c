#include "std.h"


int main(int argc, char **argv) {
	Logger *lgr = &logger_std;

	log_trace(lgr, "==> %s\n", "1");
	log_debug(lgr, "==> %s\n", "2");
	log_info(lgr, "==> %s\n", "lorem ipsum");
	log_info(lgr, "==> %s\n", "dolor sit amet");
	log_info(lgr, "==> %s\n", "consectetur adipiscing elit");
	log_info(lgr, "==> %s\n", "sed do eiusmod tempor incididunt ut labore");
	log_warn(lgr, "==> %s\n", "4");
	log_error(lgr, "==> %s\n", "5");
	log_critical(lgr, "==> %s\n", "6");
}
