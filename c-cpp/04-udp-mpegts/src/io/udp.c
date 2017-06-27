#include "udp.h"


UDP *udp_new(void) {
	UDP *it = calloc(1, sizeof(UDP));
	return it;
}

UDPResult udp_open_i(UDP *it, const char *group, const uint16_t port, char *ifn,
                     char *ebuf, size_t ebufsz) {
	UDPResult ret = UDP_RESULT_OK;
	struct ip_mreq mreq;
	char           ifnipv4[255] = { 0 };
	struct in_addr ifnaddr;

	it->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (it->sock < 0) {
		snprintf(ebuf, ebufsz, "socket error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SOCKET; goto cleanup;
	}

	bzero((char *)&it->addr, sizeof(it->addr));
	it->addr.sin_family = AF_INET;
	it->addr.sin_addr.s_addr = inet_addr(group);
	it->addr.sin_port = htons(port);
	it->addrlen = sizeof(it->addr);

	mreq.imr_multiaddr.s_addr = inet_addr(group);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(it->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "setsockopt IPPROTO-IP/IP-ADD-MEMBERSHIP error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SETSOCKOPT; goto cleanup;
	}

	if (setsockopt(it->sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "setsockopt SOL-SOCKET/SO-REUSEADDR error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SETSOCKOPT; goto cleanup;
	}

	if ((ifn) && (ifn[0] != '\0')) {
		if ((ret=udp_iftipv4(ifn, ifnipv4, ebuf, ebufsz))) goto cleanup;

		inet_aton(ifnipv4, &ifnaddr);
		if (setsockopt(it->sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ifnaddr, sizeof(ifnaddr)) < 0 ) {
			if ((ebuf) && (ebufsz))
				snprintf(ebuf, ebufsz, "setsockopt IPPROTO-IP/IP-MULTICAST-IF \"%s\" error: \"%s\"",
					ifnipv4, strerror(errno));
			ret = UDP_RESULT_ERR_SETSOCKOPT; goto cleanup;
		}
	}

	if (bind(it->sock, (struct sockaddr *)&it->addr, sizeof(it->addr)) < 0) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "bind error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_BIND; goto cleanup;
	}

cleanup:
	return ret;
}

UDPResult udp_open_o(UDP *it, const char *group, const uint16_t port, int is_mcast, char *ifn,
                     char *ebuf, size_t ebufsz) {
	int ret = 0;
	uint8_t ttl = 200;
	char           ifnipv4[255] = { 0 };
	struct in_addr ifnaddr;

	bzero((char *)&it->addr, sizeof(it->addr));
	it->addr.sin_family = AF_INET;
	it->addr.sin_addr.s_addr = inet_addr(group);
	it->addr.sin_port = htons(port);
	it->addrlen = sizeof(struct sockaddr_in);

	it->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (it->sock < 0) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "socket error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SOCKET; goto cleanup;
	}

	if (setsockopt(it->sock, IPPROTO_IP,
	    is_mcast ? IP_MULTICAST_TTL : IP_TTL,
	    &ttl, sizeof(ttl))) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "setsockopt TTL error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SOCKET; goto cleanup;
	}

	if ((ifn) && (ifn[0] != '\0')) {
		if ((ret=udp_iftipv4(ifn, ifnipv4, ebuf, ebufsz))) goto cleanup;

		inet_aton(ifnipv4, &ifnaddr);
		if (setsockopt(it->sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ifnaddr, sizeof(ifnaddr)) < 0 ) {
			if ((ebuf) && (ebufsz))
				snprintf(ebuf, ebufsz, "setsockopt IPPROTO-IP/IP-MULTICAST-IF \"%s\" error: \"%s\"",
					ifnipv4, strerror(errno));
			ret = UDP_RESULT_ERR_SETSOCKOPT; goto cleanup;
		}
	}

cleanup:
	return ret;
}

int udp_iftipv4(char *ifn, char *ipv4, char *ebuf, size_t ebufsz) {
	int ret = 0,
	    sock = 0;
	struct ifreq ifrq;

	if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "socket error: \"%s\"", strerror(errno));
		ret = UDP_RESULT_ERR_SOCKET; goto cleanup;
	}

	// AF_INET  == IPv4 address
	// AF_INET6 == IPv6 address
	ifrq.ifr_addr.sa_family = AF_INET;

	strncpy(ifrq.ifr_name, ifn, IFNAMSIZ-1);

	if (ioctl(sock, SIOCGIFADDR, &ifrq)) {
		if ((ebuf) && (ebufsz))
			snprintf(ebuf, ebufsz, "ioctl if: \"%s\" error: \"%s\"", ifn, strerror(errno));
		ret = UDP_RESULT_ERR_IOCTL; goto cleanup;
	}

	strcpy(ipv4, inet_ntoa(((struct sockaddr_in *)&ifrq.ifr_addr)->sin_addr));

cleanup:
	if (sock) {
		shutdown(sock, 2);
		close(sock);
	}

	return ret;
}

// impl Reader for UDP
int udp_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
	int ret = UDP_RESULT_OK;
	UDP *it = NULL;

	it = (UDP*)ctx;

	*n = recvfrom(it->sock, buf, bufsz, 0, (struct sockaddr *)&it->addr, &it->addrlen);
	if (n < 0) {
		fprintf(stderr, "[udp-read @ %p] recvfrom error: \"%s\"\n",
			it, strerror(errno));
		ret = UDP_RESULT_ERR_RECVFROM; goto cleanup;
	}

cleanup:
	return ret;
}

// impl Writer for UDP
int udp_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
	int ret = 0;
	UDP *it = NULL;

	it = (UDP*)ctx;

	*n = sendto(it->sock, buf, bufsz, 0, (struct sockaddr *)&it->addr, it->addrlen);
	if (n < 0) {
		fprintf(stderr, "[udp-write @ %p] sendto error: \"%s\"\n",
			it, strerror(errno));
		ret = 1; goto cleanup;
	}

cleanup:
	return ret;
}

void udp_del(UDP *it) {
	if (it) {
		shutdown(it->sock, 2);
		free(it);
	}
}
