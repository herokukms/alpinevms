#ifndef PROXY_PROTOCOL_H_
#define PROXY_PROTOCOL_H_

#ifndef _WIN32
#include <sys/socket.h>
enum {
  LIBPROXYPROTO_V1 = (1 << 0),
  LIBPROXYPROTO_V2 = (1 << 1),
};

void _proxy_protocol_init(void);
static int (*sys_close)(int fd);
static int (*sys_accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
static int (*sys_accept4)(int sockfd, struct sockaddr *addr, socklen_t *addrlen,
                          int flags);
static int (*sys_getpeername)(int sockfd, struct sockaddr *addr,
                              socklen_t *addrlen);
#pragma GCC diagnostic ignored "-Wpedantic"
int _pp_close(int fd);
int _pp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int _pp_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
int _pp_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
#pragma GCC diagnostic warning "-Wpedantic"
static int read_evt(int fd, struct sockaddr *from, socklen_t ofromlen,
                    socklen_t fromlen);

static const char v2sig[12] =
    "\x0D\x0A\x0D\x0A\x00\x0D\x0A\x51\x55\x49\x54\x0A";

static char *debug;
static char *must_use_protocol_header;
static int version = LIBPROXYPROTO_V1 | LIBPROXYPROTO_V2;

#endif
#endif
