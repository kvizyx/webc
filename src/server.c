#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include "request.h"
#include "server.h"
#include "config.h"

#define SERVER_SOCKFD_IDX 0

server_t *server_init() {
    server_t *server = (server_t*)malloc(sizeof(server_t));

    server->sockfds_len = 1;
    server->sockfds_cap = 100;

    server->sockfds = malloc(sizeof(*server->sockfds) * server->sockfds_cap);

    return server;
}

void server_start(server_t *server) {
    if (!server) return;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(SERVER_PORT);
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    server->addr = addr;
    server->fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server->fd < 0) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    server->sockfds[SERVER_SOCKFD_IDX].fd = server->fd;
    server->sockfds[SERVER_SOCKFD_IDX].events = POLLIN;

    if (bind(server->fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int on = 1;
    if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        fprintf(stderr, "set socket option: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(server->fd, SERVER_BACKLOG) == -1) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Listening on %s:%d\n", SERVER_HOST, SERVER_PORT);

    while(true) {
        int events_count = poll(server->sockfds, server->sockfds_len, -1);
        if (events_count < 0) {
            fprintf(stderr, "poll: %s\n", strerror(errno));
            continue;
        }

        // handle new connection
        if (server->sockfds[SERVER_SOCKFD_IDX].revents | POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_s = sizeof(client_addr);

            int client_fd = accept(server->fd, (struct sockaddr *)&client_addr, &client_addr_s);
            if (client_fd < 0) {
                fprintf(stderr, "accept: %s\n", strerror(errno));
                continue;
            }

            int recv_bytes = recv(client_fd, server->recv_buf, sizeof(server->recv_buf), 0);
                if (recv_bytes <= 0) {
                    close(client_fd);
                    continue;
                }

                request_t req = request_parse(server->recv_buf);
                
                // handle request

                // reset receive buffer
                memset(&server->recv_buf, 0, sizeof(server->recv_buf));            

            if (events_count == 1) continue;
        }
    }
}

void server_free(server_t *server) {
    if (!server) return;

    free(server->sockfds);
    free(server);
}