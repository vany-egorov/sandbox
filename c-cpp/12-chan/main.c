#include <stdio.h>     /* printf */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include "./chan.h"
#include "./msgs.h"


int main (int argc, char *argv[]) {
	int ret = EX_OK;

	Msgs *msgs_in = NULL;
	Msgs *msgs_out = NULL;

	printf("!\n");

cleanup:
	return ret;
}
