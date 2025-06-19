#ifndef GAME_H
#define GAME_H

#include "include/state.h"

#define GAME_STATE_ACCEPTING 1

Game game_new(server_client_t* first, server_client_t* second);
void game_accept(Game* game, server_client_t* client);
uint8_t game_accepted(Game *game);
server_client_t* game_other_player(Game* game, server_client_t* player);

#endif 
