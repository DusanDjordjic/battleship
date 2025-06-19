#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "include/state.h"
#include "include/users.h"

server_user_t* server_add_user(server_state_t* state, server_user_t user);
server_user_t* server_find_user_by_username(server_state_t* state, char* username);

server_client_t* server_find_client_by_username(server_state_t* state, char* username);

Game* server_add_game(server_state_t* state, Game game);
void server_remove_game(server_state_t* state, Game* game);

uint8_t client_logged_in(server_client_t* client);
void client_set_logged_in(server_client_t* client);
void client_clear_logged_in(server_client_t* client);

uint8_t client_looking_for_game(server_client_t* client);
void client_set_looking_for_game(server_client_t* client);
void client_clear_looking_for_game(server_client_t* client);
void client_join_game(server_client_t* client, Game* game);

#endif
