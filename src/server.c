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

#define BACKLOG_QUEUE 50
#define PORT 8080

void polls_add(struct pollfd *polls[], int fd, int *len, int *cap) {
    if (*len == *cap) {
        *cap *= 2;
        *polls = realloc(*polls, sizeof(**polls) * (*cap));
    }

    (*polls)[*len].fd = fd;
    (*polls)[*len].events = POLLIN;

    (*len)++;
}

void polls_remove(struct pollfd *polls, int polls_i, int *len) {
    polls[polls_i] = polls[*len-1];
    (*len)--;
}

int main() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(PORT);
    memset(&addr.sin_zero, 0, sizeof addr.sin_zero);
    
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    int on = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        fprintf(stderr, "set socket option: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (listen(server_fd, BACKLOG_QUEUE) == -1) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_storage client_addr;
    socklen_t client_addr_s = sizeof(client_addr);

    int polls_cap = 10;
    int polls_len = 1;
    struct pollfd *polls = malloc(sizeof(*polls) * polls_cap);

    polls[0].fd = server_fd;
    polls[0].events = POLLIN;

    char recv_buf[1024];

    printf("Server listening for incoming connections...\n");

    // serving events on socketfd's here
    while(true) {
        int events_count = poll(polls, polls_len, -1);
        if (events_count < 0) {
            fprintf(stderr, "poll: %s\n", strerror(errno));
            continue;
        }

        if (polls[0].revents & POLLIN) {
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_s);
            if (client_fd < 0) {
                fprintf(stderr, "accept: %s\n", strerror(errno));
                continue;
            }

            polls_add(&polls, client_fd, &polls_len, &polls_cap);

            printf("Client connected on socket %d\n", client_fd);

            if (events_count == 1) continue;
        }
        
        int events_processed = 0;

        for (int i = 1; i < polls_cap; ++i) {
            if (events_processed == events_count) {
                break;
            }

            if (polls[i].revents & POLLIN) {
                int recv_bytes = recv(polls[i].fd, recv_buf, sizeof(recv_buf), 0);
                if (recv_bytes <= 0) {
                    close(polls[i].fd);

                    printf("Client disconnected on socket %d\n", polls[i].fd);

                    polls_remove(polls, i, &polls_len);
                    continue;
                }

                printf("From client on socket %d: %s\n", polls[i].fd, recv_buf);

                memset(&recv_buf, 0, sizeof(recv_buf));
            }
        }
    }
    
    return EXIT_SUCCESS;
}