#include "include/game.h"
#include "include/globals.h"
#include "include/state.h"
#include "include/users.h"
#include "include/vector/vector.h"
#include <pthread.h>
#include <string.h>

server_user_t* server_add_user(server_state_t* state, server_user_t user) {
    pthread_rwlock_wrlock(&state->users_rwlock);
    vector_push(&state->users, &user);
    server_user_t* out = vector_at(&state->users, state->users.logical_length - 1);
    pthread_rwlock_unlock(&state->users_rwlock);
    return out;
}

server_user_t* server_find_user_by_username(server_state_t* state, char* username) {
    pthread_rwlock_rdlock(&state->users_rwlock);

    uint8_t found = 0;
    server_user_t* user = NULL;

    for (unsigned int i = 0; i < state->users.logical_length ; i++ ) {
         user = vector_at(&state->users, i);
         if (strncmp(username, user->username, USERNAME_MAX_LEN) == 0) {
             found = 1;
             break;
         }
    }

    if (!found) {
        user = NULL;
    }

    pthread_rwlock_unlock(&state->users_rwlock);
    return user;
}

server_client_t* server_find_client_by_username(server_state_t* state, char* username) {
    pthread_rwlock_rdlock(&state->clients_rwlock);

    uint8_t found = 0;
    server_client_t* client= NULL;

    for (unsigned int i = 0; i < state->clients.logical_length ; i++ ) {
         client = vector_at(&state->clients, i);
         if (client->user == NULL) {
             continue;
         }

         if (strncmp(username, client->user->username, USERNAME_MAX_LEN) == 0) {
             found = 1;
             break;
         }
    }

    if (!found) {
        client = NULL;
    }

    pthread_rwlock_unlock(&state->clients_rwlock);
    return client;
}


server_game_t* server_add_game(server_state_t* state, server_game_t game) {
    pthread_rwlock_wrlock(&state->games_rwlock);
    game.id = state->next_game_id;
    state->next_game_id++;

    // Try to find a free space in games vector to add the new game
    server_game_t* out = NULL;
    for (uint32_t i = 0; i < state->games.logical_length; i++) {
        server_game_t* g = vector_at(&state->games, i);
        if (g->state == GAME_STATE_CLOSED) {
            out = g;
            break;
        }
    }

    if (out == NULL) {
        vector_push(&state->games, &game);
        out = vector_at(&state->games, state->games.logical_length - 1);
    } else {
        *out = game;
    }

    pthread_rwlock_unlock(&state->games_rwlock);
    return out;
}

void server_remove_game(server_state_t* state, server_game_t* game) {
    pthread_rwlock_wrlock(&state->games_rwlock);
  
    game_close(game);

    pthread_rwlock_unlock(&state->games_rwlock);

}
uint8_t client_logged_in(server_client_t* client) {
    return client->flags & CLIENT_LOGGED_IN;
}

void client_set_logged_in(server_client_t* client) {
    client->flags |= CLIENT_LOGGED_IN;
}

void client_clear_logged_in(server_client_t* client) {
    client->flags &= ~CLIENT_LOGGED_IN;
}

uint8_t client_looking_for_game(server_client_t* client) {
    return client->flags & CLIENT_LOOKING_FOR_GAME;
}

void client_set_looking_for_game(server_client_t* client) {
    client->flags |= CLIENT_LOOKING_FOR_GAME;
}

void client_clear_looking_for_game(server_client_t* client) {
    client->flags &= ~CLIENT_LOOKING_FOR_GAME;
}

void client_join_game(server_client_t* client, server_game_t* game) {
    client->game = game;
}

game_results_t* server_add_game_result(server_state_t* state, game_results_t res) {
    pthread_rwlock_wrlock(&state->game_results_rwlock);
    vector_push(&state->game_results, &res);
    game_results_t* out = vector_at(&state->game_results, state->game_results.logical_length - 1);
    pthread_rwlock_unlock(&state->game_results_rwlock);
    return out;
}

