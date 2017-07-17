#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include "filter.h"  /* filter */


int main(int argc, char *argv[]) {
	int ret = EX_OK;

cleanup:
	return ret;
}
