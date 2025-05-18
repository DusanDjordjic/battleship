#include <include/tcp_server.h>
#include <assert.h>
#include <include/args.h>
#include <include/errors.h>
#include <include/server.h>
#include <include/state.h>
#include <include/vector/vector.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
	int sock_fd;
	struct sockaddr_in addr;

	server_state_t* state;

	char username[USERNAME_MAX_LEN];
	unsigned char logged_in;
} client_t;

void client_deinit(void* elem)
{
	fprintf(stdout, "client_deinit: %p\n", elem);
}

void* handle_client_connetion(void* params);

int main(int argc, char** argv)
{
	server_state_t state = { 0 };
	vector_create(&state.clients, sizeof(client_t));
	pthread_mutex_init(&state.clients_lock, NULL);

	error_code err = server_parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// parse_args will log the error
		return 1;
	}

	state.sock_fd = start_server(state.port);
	if (state.sock_fd == -1) {
		fprintf(stderr, "failed to start server\n");
		return 2;
	}

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int client_sock_fd = accept(state.sock_fd, (struct sockaddr*)&client_addr, &len);
		if (client_sock_fd == -1) {
			perror("failed to accept connection");
			goto EXIT;
		}

		client_t new_client = {
			.sock_fd = client_sock_fd,
			.state = &state,
			.addr = client_addr,
			.logged_in = 0,
		};

		pthread_mutex_lock(&state.clients_lock);

		vector_push(&state.clients, &new_client);
		client_t* client_p = vector_at(&state.clients, state.clients.logical_length - 1);

		pthread_mutex_unlock(&state.clients_lock);

		pthread_t client_thread;
		pthread_create(&client_thread, NULL, handle_client_connetion, client_p);
		pthread_detach(client_thread);
	}

EXIT:
	vector_destroy(&state.clients, client_deinit);
	pthread_mutex_destroy(&state.clients_lock);
	close(state.sock_fd);
	return 0;
}

void* handle_client_connetion(void* params)
{
	client_t* client = params;

	uint64_t index = ((uint64_t)client - (uint64_t)client->state->clients.elements) / client->state->clients.element_size;
	fprintf(stdout, "Running client handler SOCK: %d INDEX:%lu\n", client->sock_fd, index);

	// Send welcome message

	return NULL;
}
