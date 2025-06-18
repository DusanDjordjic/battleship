#ifndef STATE_H
#define STATE_H

#include "include/users.h"
#include "vector/vector.h"
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <stdint.h>
#include <include/globals.h>

// Client types 

typedef struct {
	int sock_fd;
	struct sockaddr_in addr;

    // User info
	client_user_t user;
    // Is user logged in
    uint8_t logged_in;
    // Api token used in requests
	char api_key[API_KEY_LEN];
    // If lobby_id is not 0 that meants that player is in game
    uint32_t lobby_id;
} client_state_t;


// Server types 

typedef struct {
    int sock_fd; 
	uint16_t port;

    // Array of clients 
    Vector clients;
    // Lock used to limit access to array
    pthread_mutex_t clients_lock;
    
    // List of all registered users. 
    // Loaded from a file at the start of program
    Vector users;
    pthread_mutex_t users_lock;
} server_state_t;

typedef struct {
    // Thread that handles client connection
    pthread_t handler_thread;

    // Back pointer to whole server state
	server_state_t* state;

    // User information about client
	server_user_t user;

	int sock_fd;
	struct sockaddr_in addr;
    char api_key[API_KEY_LEN];
    uint32_t flags;
} server_client_t;

#endif
