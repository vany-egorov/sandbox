#ifndef __APPS_3_INPUT_FILE__
#define __APPS_3_INPUT_FILE__


#include <io/file.h>  /* File */
#include <url/url.h>  /* URL */


typedef struct input_file_s InputFile;

struct input_file_s {
	File i;
};

int input_file_new(InputFile **out);


#endif /* __APPS_3_INPUT_FILE__ */
