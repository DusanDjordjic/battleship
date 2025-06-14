#include "include/users.h"
#include <asm-generic/errno-base.h>
#include <bits/types/sigset_t.h>
#include <errno.h>
#include <include/globals.h>
#include <include/messages.h>
#include <include/tcp_server.h>
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
#include <sys/select.h>

#define USERS_FILEPATH "./users.db"

void client_deinit(void* elem)
{
    client_state_t* client = elem;
    close(client->sock_fd);
	fprintf(stdout, "Closing client %d\n", client->sock_fd);
}

void* handle_client_connetion(void* params);
SignupResponseMessage handle_signup_request(server_client_t* client, const char* buffer);
void generate_random_hex_string(char* buffer, uint32_t len);

volatile sig_atomic_t interrupted = 0;

void handle_sigint(int sig) {
    interrupted = sig;
}

int main(int argc, char** argv)
{
    // On sigint set interrupted to non zero value
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

	state.sock_fd = start_server(state.port);
	if (state.sock_fd == -1) {
		fprintf(stderr, RED "ERROR: Failed to start server\n" RESET);
        return 1;
	}

    fprintf(stdout, "Server started on port %u\n", state.port);

    fd_set readfds;

    // Try until we are interrupted or an error happens 
	while (!interrupted) {

        // Select modifies readfds so we need to set it everytime
        FD_SET(state.sock_fd, &readfds);
    
        // Select will return -1 if we receive an interrupt
        int ret = select(state.sock_fd + 1, &readfds, NULL, NULL, NULL);
        if (ret == -1 && errno == EINTR) {
            break;
        } 

        if (ret == -1) {
            fprintf(stderr, "ERROR: select failed, %s\n", strerror(errno));
            break;
        }

        if (!FD_ISSET(state.sock_fd, &readfds)) {
            continue;
        }

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

    fprintf(stderr, "\nInterrupted %d: Stopping server..\n", interrupted);

    err = users_save(&state.users, USERS_FILEPATH);
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to save users" RESET, error_to_string(err));
    }

	close(state.sock_fd);
	vector_destroy(&state.clients, client_deinit);
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

        // Parse message
        uint8_t message_type = *((uint8_t*)(buffer));
        switch (message_type) {
            case MSG_SIGNUP: {
                fprintf(stdout, "CLIENT %d: Received signup request\n", client->sock_fd);
                SignupResponseMessage res = handle_signup_request(client, buffer);
                error_code err = send_message(client->sock_fd, &res, sizeof(SignupResponseMessage));
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send signup response, %d - %s\n" RESET, client->sock_fd, err, error_to_string(err));
                }

                // Save client to server state users array and to a file
                break;
            }

            case MSG_LOGIN:
            default:
                fprintf(stderr, RED "ERROR: CLIENT %d: Message type is unknown %u\n" RESET, client->sock_fd, message_type);
        }
    }

    // TODO remove client from vector

	return NULL;
}

SignupResponseMessage handle_signup_request(server_client_t* client, const char* buffer) {
    SignupRequestMessage req = *(SignupRequestMessage*)(buffer);
   
    pthread_mutex_lock(&client->state->users_lock);

    char found = 0;
    for (unsigned int i = 0; i < client->state->users.logical_length ; i++ ) {
         server_user_t* user = vector_at(&client->state->users, i);
         if (strncmp(req.username, user->username, USERNAME_MAX_LEN) == 0) {
             found = 1;
         }

         if (found) {
             break;
         }
    }
    pthread_mutex_unlock(&client->state->users_lock);

    SignupResponseMessage res = {0};
    if (found) {
        res.error.status_code = STATUS_CONFLICT;
        sprintf(res.error.message, "Username %s already exists", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User signup failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);
        return res;
    }
    
    strncpy(client->user.username, req.username, USERNAME_MAX_LEN);
    strncpy(client->user.password, req.password, PASSWORD_MAX_LEN);

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" signed up\n" RESET, client->sock_fd,client->user.username);
   
    pthread_mutex_lock(&client->state->users_lock);
    vector_push(&(client->state->users), &client->user);
    pthread_mutex_unlock(&client->state->users_lock);
    
    res.success.status_code = STATUS_OK;
    generate_random_hex_string(res.success.api_key, API_KEY_LEN);
    return res;
}

LoginResponseMessage handle_login_request(server_client_t* client, const char* buffer) {
    LoginRequestMessage req = *(LoginRequestMessage*)(buffer);
   
    pthread_mutex_lock(&client->state->clients_lock);

    char found = 0;
    server_client_t* existingClient = NULL;
    for (unsigned int i = 0; i < client->state->clients.logical_length ; i++ ) {
         existingClient = vector_at(&client->state->clients, i);
         if (strncmp(req.username, existingClient->user.username, USERNAME_MAX_LEN) == 0) {
             found = 1;
         }

         if (found) {
             break;
         }
    }
    pthread_mutex_unlock(&client->state->clients_lock);

    LoginResponseMessage res = {0};
    if (!found) {
        res.error.status_code = STATUS_NOT_FOUND;
        sprintf(res.error.message, "Failed to find user with username \"%s\"", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User signup failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);
        return res;
    }

    
    strncpy(client->user.username, req.username, USERNAME_MAX_LEN);
    strncpy(client->user.password, req.password, PASSWORD_MAX_LEN);

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" signed up\n" RESET, client->sock_fd,client->user.username);
    
    res.success.status_code = STATUS_OK;
    generate_random_hex_string(res.success.api_key, API_KEY_LEN);
    return res;
}

#define BYTE_MAX 256
void generate_random_hex_string(char* buffer, uint32_t len) {
    uint32_t half = len / 2;

    for (size_t i = 0; i < half; i++) {
        uint8_t byte = (uint8_t)(rand() % BYTE_MAX);
        sprintf(buffer + (i * 2), "%02x", byte);
    }
}

