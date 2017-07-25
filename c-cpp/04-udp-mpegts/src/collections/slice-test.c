#include <stdio.h>    /* printf */
#include <inttypes.h> /* PRIu64, PRId64 */

#include "slice.h"


typedef struct data_s Data;
struct data_s {
	uint8_t  a;
	uint16_t b;
	uint32_t c;
	uint64_t d;
	int64_t  e;
};


int main(int argc, char *argv[]) {
	Slice *s = NULL;
	Data d = { 0 };
	int i = 0;

	slice_new(&s, sizeof(Data));
	for (i = 0; i < 30; i++) {
		d.a = (uint8_t)i;
		d.b = (uint16_t)i+1;
		d.c = (uint32_t)i+2;
		d.d = (uint64_t)i+3;
		d.e = (int64_t)i+4;

		slice_append(s, &d);

		printf("[+] #%2d {a: %2d, b: %2d, c: %2d, d: %2" PRIu64 ", e: %2" PRId64 "}\n",
			i, d.a, d.b, d.c, d.d, d.e);
	}

	printf("\n");

	for (i = 0; i < 30; i++) {
		slice_get_copy_data(s, i, &d);
		printf("[~] #%2d {a: %2d, b: %2d, c: %2d, d: %2" PRIu64 ", e: %2" PRId64 "}\n",
			i, d.a, d.b, d.c, d.d, d.e);
	}

	printf("slice_del_el test\n");

	slice_del_el(s, 5);   /* - 5 */
	slice_del_el(s, 20);  /* - 21 */
	slice_del_el(s, 21);  /* - 23 */
	slice_del_el(s, s->len-1);

	printf("len: %ld\n", s->len);
	printf("cap: %ld\n", s->cap);

	for (i = 0; i < (int)s->len; i++) {
		slice_get_copy_data(s, i, &d);
		printf("[~] #%2d {a: %2d, b: %2d, c: %2d, d: %2" PRIu64 ", e: %2" PRId64 "}\n",
			i, d.a, d.b, d.c, d.d, d.e);
	}
}

