#ifndef GAME_H
#define GAME_H

#include "include/coordinate.h"
#include "include/state.h"

server_game game_new(server_client_t* first, server_client_t* second);
void game_accept(server_game* game, server_client_t* client);
server_client_t* game_other_player(server_game* game, server_client_t* player);
void game_close(server_game* game);
void game_set_clients_game_state(server_game* game, server_client_t* client, uint8_t* new_game_state);

// Status check functions
uint8_t game_closed(server_game *game);
uint8_t game_started(server_game *game);
uint8_t game_accepted(server_game* game);
uint8_t game_set_inital_turn(server_game* game, server_client_t* client);
uint8_t game_is_my_turn(server_game* game, server_client_t* client);
uint8_t game_register_shot(server_game* game, server_client_t* client, Coordinate target);
void game_next_turn(server_game* game, server_client_t* client);

#endif 
