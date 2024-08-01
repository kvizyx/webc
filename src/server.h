#ifndef __SERVER_H__
#define __SERVER_H__

#include <arpa/inet.h>
#include <poll.h>

#define SERVER_BACKLOG 50

/* Size of receive buffer in bytes */
#define SERVER_RBUF_SIZE 4096

typedef struct {
    int fd;
    struct sockaddr_in addr;

    int sockfds_cap;
    int sockfds_len;
    struct pollfd *sockfds;

    char recv_buf[SERVER_RBUF_SIZE];
} server_t;

server_t *server_init();
void server_start(server_t *server);
void server_free(server_t *server);

#endif