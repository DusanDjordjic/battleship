#include <include/globals.h>
#include <include/messages.h>
#include <errno.h>
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
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void client_deinit(void* elem)
{
	fprintf(stdout, "client_deinit: %p\n", elem);
}

void generate_random_hex_string(char* buffer, uint32_t len);

void* handle_client_connetion(void* params);
SignupResponseMessage handle_signup_request(server_client_t* client, const char* buffer);

int main(int argc, char** argv)
{
	server_state_t state = { 0 };

	error_code err = server_parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// server_parse_args will log the error
        return 1; 
    }

	state.sock_fd = start_server(state.port);
	if (state.sock_fd == -1) {
		fprintf(stderr, RED "ERROR: Failed to start server\n" RESET);
        return 2;
	}

    fprintf(stdout, "Server started on port %u\n", state.port);
    
    // Initalize state 
	vector_create(&state.clients, sizeof(server_client_t));
	pthread_mutex_init(&state.clients_lock, NULL);

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int client_sock_fd = accept(state.sock_fd, (struct sockaddr*)&client_addr, &len);
		if (client_sock_fd == -1) {
			fprintf(stderr, RED "Failed to accept connection\n" RESET);
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

	close(state.sock_fd);
	vector_destroy(&state.clients, client_deinit);
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
                return NULL;
            }

            if (err == ERR_IFD) {
                fprintf(stderr, RED "ERROR: CLIENT %d: Something is wrong with socket, closing connection\n" RESET, client->sock_fd);
                return NULL;
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
                break;
            }

            case MSG_LOGIN:
            default:
                fprintf(stderr, RED "ERROR: CLIENT %d: Message type is unknown %u\n" RESET, client->sock_fd, message_type);
        }
    }

	return NULL;
}

SignupResponseMessage handle_signup_request(server_client_t* client, const char* buffer) {
    SignupRequestMessage req = *(SignupRequestMessage*)(buffer);
   
    pthread_mutex_lock(&client->state->clients_lock);

    char found = 0;
    for (unsigned int i = 0; i < client->state->clients.logical_length ; i++ ) {
         server_client_t* existingClient = vector_at(&client->state->clients, i);
         if (strncmp(req.username, existingClient->user.username, USERNAME_MAX_LEN) == 0) {
             found = 1;
         }

         if (found) {
             break;
         }
    }
    pthread_mutex_unlock(&client->state->clients_lock);

    SignupResponseMessage res = {0};
    if (found) {
        res.error.status_code = STATUS_CONFLICT;
        sprintf(res.error.message, "Username %s already exists", req.username);
        fprintf(stderr, RED "CLIENT %d: User signup failed, %d - %s\n" RESET, client->sock_fd, res.error.status_code, res.error.message);
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

