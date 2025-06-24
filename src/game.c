
#include "include/game.h"
#include "include/globals.h"
#include "include/state.h"
#include <pthread.h>
#include <string.h>

inline uint8_t game_closed(server_game *game) {
    // If game is really closed the mutex is destroyed 
    // so we are not using the mutex here
    return game->state == GAME_STATE_CLOSED;
}

inline uint8_t game_accepted(server_game *game) {
    pthread_mutex_lock(&game->lock);
    uint8_t out = game->state == GAME_STATE_WAITING_FOR_PLAYERS_STATES;
    pthread_mutex_unlock(&game->lock);
    return out;
}

inline uint8_t game_started(server_game *game) {
    pthread_mutex_lock(&game->lock);
    uint8_t out = game->state == GAME_STATE_STARTED;
    pthread_mutex_unlock(&game->lock);
    return out;
}

server_game game_new(server_client_t* first, server_client_t* second) {
    server_game game = {
        .first = first,
        .first_accepted = 0,
        .second = second,
        .second_accepted = 0,
        .state = GAME_STATE_ACCEPTING,
    };

    pthread_mutex_init(&game.lock, NULL);

    return game;
}

void game_close(server_game* game) {
    pthread_mutex_lock(&game->lock); 

    game->first = NULL;
    game->second = NULL;
    game->first_accepted = 0;
    game->second_accepted = 0;
    game->first_state_set = 0;
    game->second_state_set = 0;
    game->id = 0;
    game->state = GAME_STATE_CLOSED;

    pthread_mutex_unlock(&game->lock); 

    pthread_mutex_destroy(&game->lock);
}

void game_accept(server_game* game, server_client_t* client) {
    pthread_mutex_lock(&game->lock);

    if (game->state != GAME_STATE_ACCEPTING) {
        UNREACHABLE;
    }

    if (game->first == client) {
        game->first_accepted = 1;
    } else if (game->second == client) {
        game->second_accepted = 1;
    }

    if (game->first_accepted && game->second_accepted) {
        game->state = GAME_STATE_WAITING_FOR_PLAYERS_STATES;
    }

    pthread_mutex_unlock(&game->lock);
}

server_client_t* game_other_player(server_game* game, server_client_t* player) {
    pthread_mutex_lock(&game->lock);

    server_client_t* out  = NULL;

    if (game->first == player) {
        out = game->second;
    } else if (game->second == player) {
        out = game->first;
    }

    pthread_mutex_unlock(&game->lock);
    return out;
}

void game_set_clients_game_state(server_game* game, server_client_t* client, uint8_t* new_game_state) {
    pthread_mutex_lock(&game->lock);
    if (game->state != GAME_STATE_WAITING_FOR_PLAYERS_STATES) {
        UNREACHABLE;
    }

    uint8_t* state = NULL;
    if (game->first == client) {
        state = game->first_game_state;
        game->first_state_set = 1;
    } else if (game->second == client) {
        state = game->second_game_state;
        game->second_state_set = 1;
    }

    memcpy(state, new_game_state, GAME_WIDTH * GAME_HEIGHT);

    if (game->first_state_set && game->second_state_set) {
        game->state = GAME_STATE_STARTED;
    }

    pthread_mutex_unlock(&game->lock);
}

