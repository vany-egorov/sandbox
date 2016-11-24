#ifndef __URL__
#define __URL__


#include <regex.h>  // regcomp, REG_EXTENDED, REG_ICASE, regex_t
#include <ctype.h>  // tolower
#include <stdio.h>  // printf
#include <stdlib.h> // atoi
#include <stdint.h> // uint16_t
#include <string.h> // strstr


#define URL_DEFAULT_HTTP_PORT  80
#define URL_DEFAULT_HTTPS_PORT 443
#define URL_DEFAULT_SSH_PORT   22

#define URL_SEPARATOR_SCHEME   "://"
#define URL_SEPARATOR_USERINFO '@'
#define URL_SEPARATOR_PORT     ':'
#define URL_SEPARATOR_PATH     '/'
#define URL_SEPARATOR_QUERY    '?'
#define URL_SEPARATOR_FRAGMENT '#'


// scheme://[userinfo@]host/path[?query][#fragment]
typedef struct url_s URL;
typedef enum url_scheme_enum URLScheme;
typedef enum url_flag_enum URLFlag;


enum url_scheme_enum {
	URL_SCHEME_UDP,
	URL_SCHEME_RTMP,
	URL_SCHEME_HTTP,
	URL_SCHEME_HTTPS,
	URL_SCHEME_WS,
	URL_SCHEME_WSS,
	URL_SCHEME_RTP,
	URL_SCHEME_FILE,
	URL_SCHEME_SSH,

	URL_SCHEME_UNKNOWN,
};

#define URL_SCHEME_UDP_DESCR      "UDP - User Datagram Protocol"
#define URL_SCHEME_RTMP_DESCR     "RTMP - Real Time Messaging Protocol"
#define URL_SCHEME_HTTP_DESCR     "HTTP - HyperText Transfer Protocol"
#define URL_SCHEME_HTTPS_DESCR    "HTTPS - HyperText Transfer Protocol Secure"
#define URL_SCHEME_WS_DESCR       "WS - WebSocket"
#define URL_SCHEME_WSS_DESCR      "WSS - WebSocket Secure"
#define URL_SCHEME_RTP_DESCR      "RTP - WebSocket Secure"
#define URL_SCHEME_FILE_DESCR     "FILE - File System"
#define URL_SCHEME_SSH_DESCR      "SSH - Secure Shell"
#define URL_SCHEME_UNKNOWN_DESCR  "UNKNOWN - unknown url scheme"

#define URL_SCHEME_UDP_STR      "udp"
#define URL_SCHEME_RTMP_STR     "rtmp"
#define URL_SCHEME_HTTP_STR     "http"
#define URL_SCHEME_HTTPS_STR    "https"
#define URL_SCHEME_WS_STR       "ws"
#define URL_SCHEME_WSS_STR      "wss"
#define URL_SCHEME_RTP_STR      "rtp"
#define URL_SCHEME_FILE_STR     "file"
#define URL_SCHEME_SSH_STR      "ssh"
#define URL_SCHEME_UNKNOWN_STR  "unk"

enum url_flag_enum {
  URL_FLAG_MULTICAST = 1, // 00000001
};

struct url_s {
	URLScheme scheme;
	char      userinfo[100];
	char      host[100];
	uint16_t  port;
	char      path[255];
	char      query[255];
	char      fragment[100];
	int       flags;
};


void url_parse(URL *it, const char *raw);
void url_sprint_json(URL *it, char *buf, size_t bufsz);
void url_sprint(URL *it, char *buf, size_t bufsz);

URLScheme url_scheme_parse(char *buf, size_t bufsz);
const char *url_scheme_string(URLScheme it);
const char *url_scheme_descr(URLScheme it);


#endif // __URL__
