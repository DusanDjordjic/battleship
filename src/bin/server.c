#include <include/users.h>
#include <include/server_handlers.h>
#include <errno.h>
#include <include/globals.h>
#include <include/messages.h>
#include <include/server.h>
#include <assert.h>
#include <include/args.h>
#include <include/errors.h>
#include <include/server.h>
#include <include/state.h>
#include <include/vector/vector.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <sys/poll.h>

#define USERS_FILEPATH "./users.db"

void* handle_client_connetion(void* params);
void generate_random_hex_string(char* buffer, uint32_t len);

volatile sig_atomic_t interrupted = 0;

void handle_sigint(int sig) {
    interrupted = sig;
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    // Need to register a signal handler so that poll will 
    // catch the signal and return EINTR
    signal(SIGINT, handle_sigint);

	server_state_t state = { 0 };
	error_code err = server_parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// server_parse_args will log the error
        return 1; 
    }

	vector_create(&state.clients, sizeof(server_client_t));
	pthread_mutex_init(&state.clients_lock, NULL);

    // Load all users from a file
    // Users load will initalize users vector
    err = users_load(&state.users, USERS_FILEPATH);
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to read users\n" RESET, error_to_string(err));
        return 1;
    }

	state.sock_fd = server_start(state.port);
	if (state.sock_fd == -1) {
		fprintf(stderr, RED "ERROR: Failed to start server\n" RESET);
        return 1;
	}

    fprintf(stdout, "Server started on port %u\n", state.port);

    struct pollfd server_poll_fd = { .fd = state.sock_fd, .events = POLLIN };

	while (!interrupted) {
        int ret = poll(&server_poll_fd, 1, -1);
        if (ret == -1 && errno == EINTR) {
            break;
        } 

        if (server_poll_fd.revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            int client_sock_fd = accept(state.sock_fd, (struct sockaddr*)&client_addr, &len);
            if (client_sock_fd == -1) {
                fprintf(stderr, RED "ERROR: Failed to accept connection\n" RESET);
                break;
            }

            server_client_t new_client =  {
                .sock_fd = client_sock_fd,
                .state = &state,
                .addr = client_addr,
                .logged_in = 0,
            };

            pthread_mutex_lock(&state.clients_lock);

            vector_push(&state.clients, &new_client);
            server_client_t* client_p = vector_at(&state.clients, state.clients.logical_length - 1);

            pthread_mutex_unlock(&state.clients_lock);

            pthread_create(&(client_p->handler_thread), NULL, handle_client_connetion, client_p);
            pthread_detach(client_p->handler_thread);
        }
	}

    fprintf(stderr, "\nStopping server..\n");

    err = users_save(&state.users, USERS_FILEPATH);
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to save users" RESET, error_to_string(err));
    }

	close(state.sock_fd);
	vector_destroy(&state.clients, NULL);
	vector_destroy(&state.users, NULL);
	pthread_mutex_destroy(&state.clients_lock);
	return 0;
}

void* handle_client_connetion(void* params)
{
	server_client_t* client = params;

	fprintf(stdout, "Running client handler SOCK: %d\n", client->sock_fd);

    char buffer[IN_BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, IN_BUFFER_SIZE);
        error_code err = read_message(client->sock_fd, buffer, IN_BUFFER_SIZE);
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to read client message, %d - %s\n" RESET, client->sock_fd, err, error_to_string(err));
            if (err == ERR_PEER_CLOSED) {
                break;
            }

            if (err == ERR_IFD) {
                fprintf(stderr, RED "ERROR: CLIENT %d: Something is wrong with socket, closing connection\n" RESET, client->sock_fd);
                break;
            }

            continue;
        }

        // Parse message type
        uint8_t message_type = *((uint8_t*)(buffer));
        switch (message_type) {
            case MSG_SIGNUP: {
                fprintf(stdout, "CLIENT %d: Received signup request\n", client->sock_fd);
                error_code err = handle_signup_request(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send signup response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }

                break;
            }

            case MSG_LOGIN: {
                fprintf(stdout, "CLIENT %d: Received login request\n", client->sock_fd);
                error_code err = handle_login_request(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send login response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }

                break;
            }
            case MSG_LOGOUT: {
                fprintf(stdout, "CLIENT %d: Received logout request\n", client->sock_fd);
                error_code err = handle_logout_request(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send login response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }

                break;
            }
            case MSG_LIST_USERS: {
                fprintf(stdout, "CLIENT %d: Received list users request\n", client->sock_fd);
                error_code err = handle_list_users(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send login response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }

                break;
            }
            default: {
                fprintf(stderr, RED "ERROR: CLIENT %d: Message type is unknown %u\n" RESET, client->sock_fd, message_type);
                error_code err = handle_unknown_request(client);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send message type unknown response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }
            }
        }
    }

    close(client->sock_fd);
    client->logged_in = 0;
    client->sock_fd = -1;

    // TODO remove client from vector
	return NULL;
}

