#include "include/game.h"
#include <include/users.h>
#include <include/server_handlers.h>
#include <include/server_utils.h>
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
void handle_client_disconnect(server_client_t* client);
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
    // TODO load games and set the next id after that
    state.next_game_id = 1;

	error_code err = server_parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// server_parse_args will log the error
        return 1; 
    }

    vector_create(&state.games, sizeof(server_game));
	vector_create(&state.clients, sizeof(server_client_t));

	pthread_rwlock_init(&state.clients_rwlock, NULL);
    pthread_rwlock_init(&state.users_rwlock, NULL);
    pthread_rwlock_init(&state.games_rwlock, NULL);

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
    
            // try to find disconnected client;
        
            pthread_rwlock_wrlock(&state.clients_rwlock);

            server_client_t* client_p = NULL;
            for (uint32_t i = 0; i < state.clients.logical_length; i++) {
                server_client_t* client = vector_at(&state.clients, i);
                if (client->sock_fd == -1) {
                    fprintf(stdout, "Found free space for client at index %d, vector len %d\n", i, state.clients.logical_length);
                    // this is free space that we can use
                    client->sock_fd = client_sock_fd;
                    client->server_state = &state;
                    client->addr = client_addr;
                    client->flags = 0;
                    client_p = client;
                    break;
                }
            }

            if (client_p == NULL) {
                // if we didn't find free space add one more
                server_client_t new_client =  {
                    .sock_fd = client_sock_fd,
                    .server_state=  &state,
                    .addr = client_addr,
                    .flags = 0,
                };
                vector_push(&state.clients, &new_client);
                client_p = vector_at(&state.clients, state.clients.logical_length - 1);
                fprintf(stdout, "Didn't find free space for client, new vector len %d\n", state.clients.logical_length);
            }

            pthread_rwlock_unlock(&state.clients_rwlock);

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
	pthread_rwlock_destroy(&state.clients_rwlock);
	pthread_rwlock_destroy(&state.users_rwlock);
	pthread_rwlock_destroy(&state.games_rwlock);

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
            if (err == ERR_PEER_CLOSED) {
                fprintf(stderr, GREEN "CLIENT %d: Disconnected\n" RESET, client->sock_fd);
                break;
            }

            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to read client message, %d - %s\n" RESET, client->sock_fd, err, error_to_string(err));
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
            case MSG_LOOK_FOR_GAME: {
                fprintf(stdout, "CLIENT %d: Received look for game request\n", client->sock_fd);
                error_code err = handle_look_for_game(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send look for game response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }
                break;
            }
            case MSG_CANCEL_LOOK_FOR_GAME: {
                fprintf(stdout, "CLIENT %d: Received cancel look for game request\n", client->sock_fd);
                error_code err = handle_cancel_look_for_game(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send cancel look for game response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }
                break;
            }
            case MSG_CHALLENGE_PLAYER: {
                fprintf(stdout, "CLIENT %d: Received challenge player request\n", client->sock_fd);
                error_code err =  handle_challenge_player(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send challenge player response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }
                break;
            }
            case MSG_CHALLENGE_ANSWER: {
                fprintf(stdout, "CLIENT %d: Received challenge answer request\n", client->sock_fd);
                error_code err = handle_challenge_answer(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send challenge answer response, %d - %s\n" RESET,
                            client->sock_fd, err, error_to_string(err));
                }
                break;
            }
            case MSG_GAME_START: {
                fprintf(stdout, "CLIENT %d: Received game start request\n", client->sock_fd);
                error_code err = handle_game_start(client, buffer);
                if (err != ERR_NONE) {
                    fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send game start response, %d - %s\n" RESET,
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

    handle_client_disconnect(client);

	return NULL;
}

void handle_client_disconnect(server_client_t* client) {
    close(client->sock_fd);
    client->user = NULL;
    client->flags = 0;
    client->sock_fd = -1;

    if (client->game == NULL) {
        return;
    }

    if (client->game->state != GAME_STATE_CLOSED) {
        // close the game, other client will get an error when he tries to 
        // send some game events 
        server_remove_game(client->server_state, client->game); 
    }
}
