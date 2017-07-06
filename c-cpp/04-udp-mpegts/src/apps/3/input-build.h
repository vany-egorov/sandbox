#ifndef __APPS_3_INPUT_BUILD__
#define __APPS_3_INPUT_BUILD__


#include <url/url.h>  /* URLProtocol */

#include "input.h"       /* Input */
#include "input-udp.h"   /* InputUDP */
#include "input-file.h"  /* InputFile */
#include "input-http-dash.h"
#include "input-http-hls.h"


int input_build(Input *it, URLProtocol protocol, InputCfg *c);


#endif /* __APPS_3_INPUT_BUILD__ */
