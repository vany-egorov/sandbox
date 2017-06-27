#include "url.h"


/* 224.0.0.0-224.0.0.255:     "Reserved for special 'well-known' multicast addresses."
 * 224.0.1.0-238.255.255.255: "Globally-scoped (Internet-wide) multicast addresses."
 * 239.0.0.0-239.255.255.255: "Administratively-scoped (local) multicast addresses."
 */
static char *URL_REG_PATTERN_UDP_MCAST_GROUP =
	"2(2[4-9]|3[0-9])"
	"(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]))"
	"(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]))"
	"(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]))";


static inline void prepchr(char *s, const char c) {
	memmove(s+1, s, strlen(s));
	s[0] = c;
}


static inline char* buf_tail(URL *it) { return &it->buf[it->buf_len]; }


static inline void path_ensure_start_slash(URL *it) {
	if (it->scheme == URL_SCHEME_FILE) { // add missing /

		if ((it->got_path) &&                           /* path was parsed */
		    (url_path(it)[0] != URL_SEPARATOR_PATH) &&  /* missing / */
		    (sizeof(it->buf) > it->buf_len+1))          /* have enought space */
			prepchr(&it->buf[it->pos_path], URL_SEPARATOR_PATH);

	}
}


static inline void split_host_port(const char *buf, const size_t bufsz, char *host, uint8_t *got_host, uint16_t *port) {
	char *token = NULL;
	char port_raw[6] = { 0 };

	/* /221.1.1.1:5500 */
	if (!bufsz) return;

	*got_host = 1;

	token = strchr(buf, URL_SEPARATOR_PORT);
	if (token) { /* host + port */
		strncpy(host, buf, token-buf);
		strncpy(
			port_raw,

			token+1,      /* port buffer start */

			              /* port buffer size =     */
			bufsz         /*   total-buffer-size    */
			- (token-buf) /*   - (host-buffer-size) */
		);
		*port = (uint16_t)atoi(port_raw);

		return;
	}

	*port = 0;
	strncpy(host, buf, bufsz);
}

void url_parse(URL *it, const char *raw) {
	char copy[300];
	char *cursor = NULL,
	     *token = NULL;
	uint8_t got_host = 0;
	size_t bufsz = 0;
	regex_t reg = { 0 }; /* UDP mcast regexp */

	memset(it, 0, sizeof(*it));
	cursor = strcpy(copy, raw);

	it->scheme = URL_SCHEME_UNKNOWN;

	if ((token=strstr(cursor, URL_SEPARATOR_SCHEME))) {
		it->scheme = url_scheme_parse(
			cursor,       /* scheme buffer */
			token-cursor  /* scheme buffer size */
		);
		cursor = token + strlen(URL_SEPARATOR_SCHEME);
	}

	if ((token=strchr(cursor, URL_SEPARATOR_USERINFO))) {
		bufsz = token-cursor;
		strncpy(
			buf_tail(it),
			cursor,  /* userinfo buffer */
			bufsz    /* userinfo buffer size */
		);

		it->pos_userinfo = it->buf_len;
		it->buf_len += (uint16_t)bufsz + 1;
		it->got_user_info = 1;

		cursor = token + 1;
	}

	if (it->scheme != URL_SCHEME_FILE) {
		if ((token=strchr(cursor, URL_SEPARATOR_PATH))) { /* got some data after host:[port]? */
			bufsz = token-cursor;

			/* ensure host != "." && host != ".." */
			if (!(((bufsz == 1) &&
			       (cursor[0] == URL_RELATIVE_PATH_START)) ||  /* '.' */
			      ((bufsz == 2) &&
			       (cursor[0] == URL_RELATIVE_PATH_START) &&
			       (cursor[1] == URL_RELATIVE_PATH_START)))) {  /* ".." */

			  split_host_port(
					cursor,  /* host[+port] buffer */
					bufsz,   /* host[+port] buffer size */
					buf_tail(it),
					&got_host,
					&it->port
				);

				if (got_host) {
					it->pos_host = it->buf_len;
					it->buf_len += strlen(&it->buf[it->pos_host]) + 1;
					it->got_host = 1;
				}

				cursor = token;
			}
		} else {
			bufsz = strlen(cursor);
			split_host_port(cursor, bufsz, buf_tail(it), &got_host, &it->port);

			if (got_host) {
				it->pos_host = it->buf_len;
				it->buf_len += strlen(&it->buf[it->pos_host]) + 1;
				it->got_host = 1;
			}

			cursor = NULL;
		}
	}

	if (cursor) {
		if ((token=strchr(cursor, URL_SEPARATOR_QUERY))) {
			bufsz = token-cursor;
			strncpy(
				buf_tail(it),
				cursor,  /* path buffer */
				bufsz    /* path buffer size */
			);

			it->pos_path = it->buf_len;
			it->buf_len += (uint16_t)bufsz + 1;
			it->got_path = 1;
			path_ensure_start_slash(it);

			cursor = token + 1;
		} else {

			if ((token=strchr(cursor, URL_SEPARATOR_FRAGMENT))) {
				bufsz = token-cursor;
				strncpy(
					buf_tail(it),
					cursor,   /* path buffer */
					bufsz     /* path buffer size */
				);

				it->pos_path = it->buf_len;
				it->buf_len += (uint16_t)bufsz + 1;
				it->got_path = 1;
				path_ensure_start_slash(it);

				cursor = token + 1;

				strcpy(buf_tail(it), cursor);
				it->pos_fragment = it->buf_len;
				it->buf_len += (uint16_t)strlen(cursor) + 1;
				it->got_fragment = 1;
			} else {
				strcpy(buf_tail(it), cursor);
				it->pos_path = it->buf_len;
				it->buf_len += (uint16_t)strlen(cursor) + 1;
				it->got_path = 1;
				path_ensure_start_slash(it);
			}

			cursor = NULL;
		}
	}

	if (cursor) {
		if ((token=strchr(cursor, URL_SEPARATOR_FRAGMENT))) {
			bufsz = token-cursor;
			strncpy(
				buf_tail(it),
				cursor,     /* query buffer */
				bufsz       /* query buffer size */
			);

			it->pos_query = it->buf_len;
			it->buf_len += (uint16_t)bufsz + 1;
			it->got_query = 1;

			cursor = token + 1;

			strcpy(buf_tail(it), cursor);
			it->pos_fragment = it->buf_len;
			it->buf_len += (uint16_t)strlen(cursor) + 1;
			it->got_fragment = 1;
		} else {
			strcpy(buf_tail(it), cursor);
			it->pos_query = it->buf_len;
			it->buf_len += (uint16_t)strlen(cursor) + 1;
			it->got_query = 1;

			cursor = NULL;
		}
	}

	{ /* post-processing: guess missing from context */

		if (it->scheme == URL_SCHEME_UNKNOWN) { /* guess scheme from context */

			if ((it->got_host) && /* host provided */
			    (url_host(it)[0] == '2'))
				it->scheme = URL_SCHEME_UDP;

			if ((!it->got_host) &&  /* no host provided */
					(it->got_path) &&   /* path provided */
					(
					  (url_path(it)[0] == '/') ||
					  (url_path(it)[0] == '.')
					))
				it->scheme = URL_SCHEME_FILE;
		}

		if ((it->scheme == URL_SCHEME_HTTP) &&
		    (!it->port))
			it->port = URL_DEFAULT_HTTP_PORT;

		if ((it->scheme == URL_SCHEME_HTTPS) &&
		    (!it->port))
			it->port = URL_DEFAULT_HTTPS_PORT;

		if ((it->scheme == URL_SCHEME_SSH) &&
		    (!it->port))
			it->port = URL_DEFAULT_SSH_PORT;

		if ((it->scheme == URL_SCHEME_UDP) &&
			  (it->got_host)) {

			// must-compile
			if (!regcomp(&reg, URL_REG_PATTERN_UDP_MCAST_GROUP, REG_EXTENDED|REG_ICASE)) {
				// must-exec
				if (!regexec(&reg, url_host(it), (size_t)0, NULL, 0))
					it->flags |= URL_FLAG_MULTICAST;
				regfree(&reg);
			}
		}
	}
}

const char* url_user_info(URL *it) { return it->got_user_info ? &it->buf[it->pos_userinfo] : ""; }
const char* url_host(URL *it)      { return it->got_host ? &it->buf[it->pos_host] : ""; }
const char* url_path(URL *it)      { return it->got_path ? &it->buf[it->pos_path] : ""; }
const char* url_query(URL *it)     { return it->got_query ? &it->buf[it->pos_query] : ""; }
const char* url_fragment(URL *it)  { return it->got_fragment ? &it->buf[it->pos_fragment] : ""; }

void url_sprint(URL *it, char *buf, size_t bufsz) {
	snprintf(buf+strlen(buf), bufsz-strlen(buf), "%s%s", url_scheme_string(it->scheme), URL_SEPARATOR_SCHEME);
	if (it->got_user_info)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%s%c", url_user_info(it), URL_SEPARATOR_USERINFO);
	if (it->got_host)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%s", url_host(it));
	if (it->port)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%c%d", URL_SEPARATOR_PORT, it->port);
	if (it->got_path)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%s", url_path(it));
	if (it->got_query)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%c%s", URL_SEPARATOR_QUERY, url_query(it));
	if (it->got_fragment)
		snprintf(buf+strlen(buf), bufsz-strlen(buf), "%c%s", URL_SEPARATOR_FRAGMENT, url_fragment(it));
}

void url_sprint_json(URL *it, char *buf, size_t bufsz) {
	snprintf(buf, bufsz, "{"
		"\"scheme\": \"%s\""
		", \"userinfo\": \"%s\""
		", \"host\": \"%s\""
		", \"port\": %d"
		", \"path\": \"%s\""
		", \"query\": \"%s\""
		", \"fragment\": \"%s\""
		", \"multicast?\": %d"
		"}",
		url_scheme_string(it->scheme),
		url_user_info(it),
		url_host(it),
		it->port,
		url_path(it),
		url_query(it),
		url_fragment(it),
		it->flags & URL_FLAG_MULTICAST
	);
}

URLScheme url_scheme_parse(char *buf, size_t bufsz) {
	int i = 0;
	char *cursor = buf;
	for (i = 0; i < bufsz; i++) {
		*cursor = tolower(*cursor);
		cursor++;
	}

	if (!strncmp(buf, "udp", bufsz)) {
		return URL_SCHEME_UDP;
	} else if (!strncmp(buf, "rtmp", bufsz)) {
		return URL_SCHEME_RTMP;
	} else if (!strncmp(buf, "http", bufsz)) {
		return URL_SCHEME_HTTP;
	} else if (!strncmp(buf, "https", bufsz)) {
		return URL_SCHEME_HTTPS;
	} else if (!strncmp(buf, "ws", bufsz)) {
		return URL_SCHEME_WS;
	} else if (!strncmp(buf, "wss", bufsz)) {
		return URL_SCHEME_WSS;
	} else if (!strncmp(buf, "rtp", bufsz)) {
		return URL_SCHEME_RTP;
	} else if (!strncmp(buf, "file", bufsz)) {
		return URL_SCHEME_FILE;
	} else if (!strncmp(buf, "ssh", bufsz)) {
		return URL_SCHEME_SSH;
	}

	return URL_SCHEME_UNKNOWN;
}

const char *url_scheme_descr(URLScheme it) {
	switch (it) {
	case URL_SCHEME_UDP:     return URL_SCHEME_UDP_DESCR;
	case URL_SCHEME_RTMP:    return URL_SCHEME_RTMP_DESCR;
	case URL_SCHEME_HTTP:    return URL_SCHEME_HTTP_DESCR;
	case URL_SCHEME_HTTPS:   return URL_SCHEME_HTTPS_DESCR;
	case URL_SCHEME_WS:      return URL_SCHEME_WS_DESCR;
	case URL_SCHEME_WSS:     return URL_SCHEME_WSS_DESCR;
	case URL_SCHEME_RTP:     return URL_SCHEME_RTP_DESCR;
	case URL_SCHEME_FILE:    return URL_SCHEME_FILE_DESCR;
	case URL_SCHEME_SSH:     return URL_SCHEME_SSH_DESCR;
	case URL_SCHEME_UNKNOWN: return URL_SCHEME_UNKNOWN_DESCR;
	}

	return URL_SCHEME_UNKNOWN_DESCR;
}

const char *url_scheme_string(URLScheme it) {
	switch (it) {
	case URL_SCHEME_UDP:     return URL_SCHEME_UDP_STR;
	case URL_SCHEME_RTMP:    return URL_SCHEME_RTMP_STR;
	case URL_SCHEME_HTTP:    return URL_SCHEME_HTTP_STR;
	case URL_SCHEME_HTTPS:   return URL_SCHEME_HTTPS_STR;
	case URL_SCHEME_WS:      return URL_SCHEME_WS_STR;
	case URL_SCHEME_WSS:     return URL_SCHEME_WSS_STR;
	case URL_SCHEME_RTP:     return URL_SCHEME_RTP_STR;
	case URL_SCHEME_FILE:    return URL_SCHEME_FILE_STR;
	case URL_SCHEME_SSH:     return URL_SCHEME_SSH_STR;
	case URL_SCHEME_UNKNOWN: return URL_SCHEME_UNKNOWN_STR;
	}

	return URL_SCHEME_UNKNOWN_STR;
}
