#include "tcp_server.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#define ACCEPT_QUEUE_LEN 128
int start_server(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		perror("failed to open server socket");
		return -1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("failed to bind server socket");
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, ACCEPT_QUEUE_LEN) < 0) {
		perror("failed to set server socket to listen mode");
		close(server_fd);
		return -1;
	}

	return server_fd;
}
