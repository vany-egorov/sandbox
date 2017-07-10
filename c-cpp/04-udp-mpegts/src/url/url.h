#ifndef __URL__
#define __URL__


/* https://url.spec.whatwg.org */


#include <regex.h>  /* regcomp, REG_EXTENDED, REG_ICASE, regex_t */
#include <ctype.h>  /* tolower */
#include <stdio.h>  /* printf */
#include <stdlib.h> /* atoi */
#include <stdint.h> /* uint16_t */
#include <string.h> /* strstr */


#define URL_DEFAULT_HTTP_PORT  80
#define URL_DEFAULT_HTTPS_PORT 443
#define URL_DEFAULT_SSH_PORT   22

#define URL_SEPARATOR_SCHEME   "://"
#define URL_SEPARATOR_USERINFO '@'
#define URL_SEPARATOR_PORT     ':'
#define URL_SEPARATOR_PATH     '/'
#define URL_SEPARATOR_QUERY    '?'
#define URL_SEPARATOR_FRAGMENT '#'

#define URL_RELATIVE_PATH_START '.'
#define URL_PATH_EXT_START '.'


// scheme://[userinfo@]host/path[?query][#fragment]
typedef struct url_s URL;
typedef enum {
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
} URLScheme;
typedef URLScheme URLProtocol;
typedef enum {
  URL_FLAG_MULTICAST = 0x01,
} URLFlag;


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

struct url_s {
	URLScheme scheme;
	uint16_t  port;

	char     buf[255];
	uint16_t buf_len;
	uint8_t
		got_user_info       :1,
		got_host            :1,
		got_path            :1,
		got_ext             :1,
		got_query           :1,
		got_fragment        :1,
		reserved_bit_fields :2;
	uint16_t
		pos_userinfo,
		pos_host,
		pos_path,
		pos_ext,
		pos_query,
		pos_fragment;
	uint8_t flags;
};


void url_parse(URL *it, const char *raw);
void url_sprint_json(URL *it, char *buf, size_t bufsz);
void url_sprint(URL *it, char *buf, size_t bufsz);

const char* url_user_info(URL *it);
const char* url_host(URL *it);
const char* url_path(URL *it);
const char* url_ext(URL *it);
const char* url_query(URL *it);
const char* url_fragment(URL *it);
const URLScheme url_scheme(URL *it);
const URLProtocol url_protocol(URL *it);

URLScheme url_scheme_parse(char *buf, size_t bufsz);
const char *url_scheme_string(URLScheme it);
const char *url_scheme_descr(URLScheme it);


#endif // __URL__
