#ifndef __UDP__
#define __UDP__


#ifndef _BSD_SOURCE
#define _BSD_SOURCE     /* Needed for using struct ip_mreq with recent glibc */
#endif

#include <stdio.h>      // printf, fprintf, snprintf
#include <errno.h>      // errno
#include <string.h>     // strerror
#include <stdlib.h>     // calloc
#include <stdint.h>     // uint8_t, uint16_t
#include <unistd.h>     // close
#include <net/if.h>     // struct ifreq
#include <arpa/inet.h>  // inet_addr
#include <sys/ioctl.h>  // struct ifreq, ioctl
#include <netinet/in.h> // struct ip_mreq


typedef struct udp_s UDP;
typedef enum udp_result_enum UDPResult;

enum udp_result_enum {
	UDP_RESULT_OK,

	UDP_RESULT_ERR_SOCKET,
	UDP_RESULT_ERR_SETSOCKOPT,
	UDP_RESULT_ERR_BIND,
	UDP_RESULT_ERR_RECVFROM,
	UDP_RESULT_ERR_IOCTL,
};

struct udp_s {
	int sock;

	socklen_t          addrlen;
	struct sockaddr_in addr;
};

UDP *udp_new(void);
UDPResult udp_connect_i(UDP *it, const char *group, const uint16_t port, char *ifn,
                        char *ebuf, size_t ebufsz);
UDPResult udp_connect_o(UDP *it, const char *group, const uint16_t port, int is_mcast, char *ifn,
                        char *ebuf, size_t ebufsz);
// interface name to IPv4 address
//
// - "ifn"  => interface name
// - "ipv4" => IPv4 address
int udp_iftipv4(char *ifn, char *ipv4, char *ebuf, size_t ebufsz);
// impl Reader for UDP
int udp_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n);
// impl Writer for UDP
int udp_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *n);
void udp_del(UDP *it);


#endif // __UDP__
