
#include "include/game.h"
#include "include/globals.h"
#include "include/state.h"

ServerGame game_new(server_client_t* first, server_client_t* second) {
    ServerGame game = {
        .first = first,
        .first_accepted = 0,
        .second = second,
        .second_accepted = 0,
        .game_state = GAME_STATE_ACCEPTING,
    };

    return game;
}

void game_accept(ServerGame* game, server_client_t* client) {
    if (game->first == client) {
        game->first_accepted = 1;
        return;
    }

    if (game->second == client) {
        game->second_accepted = 1;
        return;
    }

    UNREACHABLE;
}

uint8_t game_accepted(ServerGame *game) {
    return game->first_accepted && game->second_accepted;
}

server_client_t* game_other_player(ServerGame* game, server_client_t* player) {
    if (game->first == player) {
        return game->second;
    }

    if (game->second == player) {
        return game->first;
    }

    UNREACHABLE;
}
