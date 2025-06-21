#ifndef GAME_H
#define GAME_H

#include "include/state.h"

ServerGame game_new(server_client_t* first, server_client_t* second);
void game_accept(ServerGame* game, server_client_t* client);
uint8_t game_accepted(ServerGame *game);
server_client_t* game_other_player(ServerGame* game, server_client_t* player);

#endif 
