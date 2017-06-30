static int open(URL *url) {
}

static int read(void) {

}

static int close(void) {

}

int input_file_new(Input *input) {

}

InputVt input_file_vt {
	.open = open,
	.read = read,
	.close = close,
}
