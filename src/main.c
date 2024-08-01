#include <stdlib.h>

#include "server.h"

int main() {
    server_t *server = server_init();

    server_start(server);

    server_free(server);

    return EXIT_SUCCESS;
}