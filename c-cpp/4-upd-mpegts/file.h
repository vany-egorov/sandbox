#ifndef __IO_FILE__
#define __IO_FILE__


#include <errno.h>  // errno
#include <stdio.h>  // FILE
#include <stdlib.h> // calloc
#include <stdint.h> // uint8_t, uint16_t


typedef struct file_s File;
typedef enum file_result_enum FileResult;

enum file_result_enum {
	FILE_RESULT_OK,

	FILE_RESULT_ERR_FOPEN,
};

struct file_s {
	FILE *file;
};

File *file_new(void);
FileResult file_open(File *it, const char *path, const char *mode,
                     char *ebuf, size_t ebufsz);
// impl Reader for File
int file_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n);
// impl Writer for File
int file_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *n);
void file_del(File *it);


#endif // __IO_FILE__
