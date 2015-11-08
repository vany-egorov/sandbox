#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // close
#include <fcntl.h>      // open
#include <string.h>     // memset
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>


#define EXAMPLE_PORT 5500
#define EXAMPLE_GROUP "239.1.1.1"
#define SYNC_BYTE 0x47
#define MESSAGE_SIZE 2000


int main (int argc, char *argv[]) {
  struct sockaddr_in addr;
  int addrlen, sock, cnt, i;
  struct ip_mreq mreq;
  char message[MESSAGE_SIZE];

  /* set up socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
   perror("socket");
   exit(1);
  }
  bzero((char *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(EXAMPLE_PORT);
  addrlen = sizeof(addr);

  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }
  mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    perror("setsockopt mreq");
    exit(1);
  }

  while (1) {
    cnt = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &addr, &addrlen);
    if (cnt < 0) {
      perror("recvfrom");
      exit(1);
    } else if (cnt == 0) {
      break;
    }

    for (i = 0; i < MESSAGE_SIZE; i++) {
      if (message[i] == SYNC_BYTE) {
        printf("SYNC_BYTE got\n");
      }
    }

    printf("%s: cnt=%d message = \"%s\"\n", inet_ntoa(addr.sin_addr), cnt, message);
  }
}
