#include "include/globals.h"
#include <include/tcp_server.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#define ACCEPT_QUEUE_LEN 128
int start_server(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		fprintf(stderr, RED "ERROR: Failed to open server socket\n" RESET);
		return -1;
	}

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        fprintf(stderr, RED "ERROR: Failed to set setsockopt SO_REUSEADDR\n" RESET);
        close(server_fd);
        return -1;
    }

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		fprintf(stderr, RED "ERROR: Failed to bind server socket\n" RESET);
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, ACCEPT_QUEUE_LEN) < 0) {
		fprintf(stderr, RED "ERROR: Failed to set server socket to listen mode\n" RESET);
		close(server_fd);
		return -1;
	}

	return server_fd;
}
