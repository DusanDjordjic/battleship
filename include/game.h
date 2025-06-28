#ifndef GAME_H
#define GAME_H

#include "include/coordinate.h"
#include "include/game_results.h"
#include "include/state.h"

server_game_t game_new(server_client_t* first, server_client_t* second);
void game_accept(server_game_t* game, server_client_t* client);
server_client_t* game_other_player(server_game_t* game, server_client_t* player);
void game_close(server_game_t* game);
void game_set_clients_game_state(server_game_t* game, server_client_t* client, uint8_t* new_game_state);

// Status check functions
uint8_t game_closed(server_game_t *game);
uint8_t game_started(server_game_t *game);
uint8_t game_accepted(server_game_t* game);
uint8_t game_set_inital_turn(server_game_t* game, server_client_t* client);
uint8_t game_is_my_turn(server_game_t* game, server_client_t* client);
uint8_t game_register_shot(server_game_t* game, server_client_t* client, Coordinate target);
void game_next_turn(server_game_t* game, server_client_t* client);
// 1 of the passed client won, 0 if no one won, -1 is other client won
uint8_t game_check_win(server_game_t* game, server_client_t* client);
uint8_t game_finished(server_game_t* game);
void game_finish(server_game_t* game, server_client_t* client);
game_results_t game_create_result(server_game_t* game);
#endif 
