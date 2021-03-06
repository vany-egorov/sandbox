#include "file.h"


File *file_new(void) {
	File *it = calloc(1, sizeof(File));
	return it;
}

FileResult file_open(File *it, const char *path, const char *mode,
                     char *ebuf, size_t ebufsz) {
	FileResult ret = FILE_RESULT_OK;
	it->file = fopen(path, mode);
	if (!it->file) {
		snprintf(ebuf, ebufsz, "file open error: \"%s\"", strerror(errno));
		ret = FILE_RESULT_ERR_FOPEN; goto cleanup;
	}

cleanup:
	return ret;
}

int file_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
	FileResult ret = FILE_RESULT_OK;
	File *it = NULL;

	it = (File*)ctx;

	*n = fread((void*)buf, 1, bufsz, it->file);
	if (*n != bufsz) {
		if (feof(it->file)) {
			ret = FILE_RESULT_OK_EOF; goto cleanup;
		} else {
			fprintf(stderr, "[file-read @ %p] fread error: \"%s\"\n",
				it, strerror(errno));
			ret = FILE_RESULT_ERR_READ; goto cleanup;
		}
	}

cleanup:
	return ret;
}

int file_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
	int ret = 0;
	File *it = NULL;

	it = (File*)ctx;

	*n = fwrite(buf, sizeof(uint8_t), bufsz, it->file);
	if (*n != bufsz) {
		fprintf(stderr, "[file-write @ %p] fwrite error: \"%s\"\n",
			it, strerror(errno));
		ret = 1; goto cleanup;
	}

cleanup:
	return ret;
}

int file_seek_start(File *it) {
	fseek(it->file, 0, SEEK_SET);
}

void file_del(File *it) {
	if (!it) return;
	if (it->file) {
		fclose(it->file);
		it->file = NULL;
	}
	free(it);
}
