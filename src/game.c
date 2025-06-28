
#include "include/game.h"
#include "include/globals.h"
#include "include/state.h"
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

inline uint8_t game_closed(server_game_t *game) {
    // If game is really closed the mutex is destroyed 
    // so we are not using the mutex here
    return game->state == GAME_STATE_CLOSED;
}

inline uint8_t game_accepted(server_game_t *game) {
    pthread_mutex_lock(&game->lock);
    uint8_t out = game->state == GAME_STATE_WAITING_FOR_PLAYERS_STATES;
    pthread_mutex_unlock(&game->lock);
    return out;
}

inline uint8_t game_started(server_game_t *game) {
    pthread_mutex_lock(&game->lock);
    uint8_t out = game->state == GAME_STATE_STARTED;
    pthread_mutex_unlock(&game->lock);
    return out;
}

server_game_t game_new(server_client_t* first, server_client_t* second) {
    server_game_t game = {
        .first = first,
        .first_accepted = 0,
        .second = second,
        .second_accepted = 0,
        .state = GAME_STATE_ACCEPTING,
    };

    pthread_mutex_init(&game.lock, NULL);

    return game;
}

void game_close(server_game_t* game) {
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

void game_accept(server_game_t* game, server_client_t* client) {
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

server_client_t* game_other_player(server_game_t* game, server_client_t* player) {
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

void game_set_clients_game_state(server_game_t* game, server_client_t* client, uint8_t* new_game_state) {
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

// Returns 1 if the turn is of the passed client and 0 otherwise
inline uint8_t game_set_inital_turn(server_game_t* game, server_client_t* client) {
    if (!game_started(game)) {
        UNREACHABLE;
    }

    pthread_mutex_lock(&game->lock);

    uint8_t out = 0;

    game->turn = GAME_FIRSTS_TURN;

    if (game->first == client) {
        out = 1;
    } 

    pthread_mutex_unlock(&game->lock);

    return out;
}

// Sets the turn to current the other player
inline void game_next_turn(server_game_t* game, server_client_t* client) {
    if (!game_started(game)) {
        UNREACHABLE;
    }

    pthread_mutex_lock(&game->lock);

    if (game->first == client) {
        game->turn = GAME_SECONDS_TURN;
    } else {
        game->turn = GAME_FIRSTS_TURN;
    }

    pthread_mutex_unlock(&game->lock);
}

uint8_t game_is_my_turn(server_game_t* game, server_client_t* client) {
    if (!game_started(game)) {
        UNREACHABLE;
    }

    pthread_mutex_lock(&game->lock);

    uint8_t out  = 0;

    if (game->first == client && game->turn == GAME_FIRSTS_TURN) {
        out = 1;
    } else if (game->second == client && game->turn == GAME_SECONDS_TURN) {
        out = 1;
    }

    pthread_mutex_unlock(&game->lock);

    return out;
}

// Registeres a shot chainging the game state
// returning the state of the field before the changes 
uint8_t game_register_shot(server_game_t* game, server_client_t* client, Coordinate target) {
    if (!game_started(game)) {
        UNREACHABLE;
    }

    pthread_mutex_lock(&game->lock);

    uint8_t out = 0;
    uint8_t* target_board = NULL;

    if (game->first == client) {
        target_board = game->second_game_state;
    } else {
        target_board = game->first_game_state;
    }

    int8_t index = target.x + target.y * GAME_WIDTH;
    if (index < 0 && index >= GAME_WIDTH * GAME_HEIGHT) {
        out = GAME_FIELD_INVALID;
    } else {
        out = target_board[index];
        switch (out) {
            case GAME_FIELD_EMPTY:
                target_board[index] = GAME_FIELD_MISS;
                break;
            case GAME_FIELD_SHIP:
                target_board[index] = GAME_FIELD_HIT;
                break;
            case GAME_FIELD_MISS:
                break;
            case GAME_FIELD_HIT:
                break;
            default:
                UNREACHABLE;
        }
    }

    pthread_mutex_unlock(&game->lock);

    return out;
}

uint8_t game_check_win(server_game_t* game, server_client_t* client) {
    if (!game_started(game)) {
        UNREACHABLE;
    }

    int8_t out = 0;
    pthread_mutex_lock(&game->lock);

    uint8_t* opponents_board = NULL;

    if (game->first == client) {
        opponents_board = game->second_game_state;
    } else {
        opponents_board = game->first_game_state;
    }
   
    uint8_t opponents_ships = 0;
    for (uint16_t i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
        if (opponents_board[i] == GAME_FIELD_SHIP) {
            opponents_ships++;
            break;
        }
    }

    // If opponent doesn't have any ships left we won
    if (opponents_ships == 0) {
        out = 1;
    } 

    pthread_mutex_unlock(&game->lock);

    return out;
}

uint8_t game_finished(server_game_t* game) {
    pthread_mutex_lock(&game->lock);
    uint8_t out = game->state == GAME_STATE_FINISHED;
    pthread_mutex_unlock(&game->lock);
    return out;
}

void game_finish(server_game_t* game, server_client_t* client) {
    pthread_mutex_lock(&game->lock);
    game->state = GAME_STATE_FINISHED;

    if (game->first == client) {
        game->won = GAME_FIRST_WON;
    } else {
        game->won = GAME_SECOND_WON;
    }

    pthread_mutex_unlock(&game->lock);
}


game_results_t game_create_result(server_game_t* game) {
    if (!game_finished(game)) {
        UNREACHABLE;
    }

    pthread_mutex_lock(&game->lock);

    game_results_t res;
    res.won = game->won;
    memcpy(res.first_game_state, game->first_game_state, GAME_WIDTH * GAME_HEIGHT);
    memcpy(res.second_game_state, game->second_game_state, GAME_WIDTH * GAME_HEIGHT);
    strncpy(res.first_player_username, game->first->user->username, USERNAME_MAX_LEN);
    strncpy(res.second_player_username, game->second->user->username, USERNAME_MAX_LEN);
    pthread_mutex_unlock(&game->lock);

    return res;
}
