#ifndef STATE_H
#define STATE_H

#include "include/users.h"
#include "include/globals.h"
#include "include/vector/vector.h"
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <stdint.h>

typedef struct server_game_t server_game_t;

typedef struct {
    uint32_t game_id;
    uint8_t my_state[GAME_WIDTH * GAME_HEIGHT];
    uint8_t opponents_state[GAME_WIDTH * GAME_HEIGHT];
} client_game_t;

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
    client_game_t game;
} client_state_t;


// Server types 

typedef struct {
    int sock_fd; 
	uint16_t port;

    // Array of clients 
    Vector clients;
    // Lock used to limit access to array
    pthread_rwlock_t clients_rwlock;
  
    Vector games;
    pthread_rwlock_t games_rwlock;
    uint32_t next_game_id;

    // List of all registered users. 
    // Loaded from a file at the start of program
    Vector users;
    // Server initalizes users.
    // Clients can trigger signup which will modify the users 
    // Clients can trigger login which needs to read the users
    pthread_rwlock_t users_rwlock;


    // Finished games are added to the game results
    Vector game_results;
    pthread_rwlock_t game_results_rwlock;
} server_state_t;

typedef struct {
    // Thread that handles client connection
    pthread_t handler_thread;

    // Back pointer to whole server state
	server_state_t* server_state;

    // User information about client
	server_user_t* user;

	int sock_fd;
	struct sockaddr_in addr;
    char api_key[API_KEY_LEN];
    uint32_t flags;
    server_game_t* game;
} server_client_t;

struct server_game_t {
    uint32_t id;

    server_client_t* first;
    server_client_t* second;

    uint8_t first_accepted;
    uint8_t second_accepted;

    // General game state like started, waiting, etc..
    uint8_t state;

    // prevent concurrent access from both players at the same time
    pthread_mutex_t lock;

    uint8_t first_game_state[GAME_WIDTH * GAME_HEIGHT];
    uint8_t second_game_state[GAME_WIDTH * GAME_WIDTH];
    uint8_t first_state_set;
    uint8_t second_state_set;

    uint8_t turn;
    uint8_t won;
};

#endif
