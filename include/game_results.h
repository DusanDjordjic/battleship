#ifndef GAME_RESULTS_H 
#define GAME_RESULTS_H

#include "include/errors.h"
#include "include/globals.h"
#include "include/vector/vector.h"

typedef struct {
    char first_player_username[USERNAME_MAX_LEN];
    char second_player_username[USERNAME_MAX_LEN];
    uint8_t won;
    uint8_t first_game_state[GAME_WIDTH * GAME_HEIGHT];
    uint8_t second_game_state[GAME_WIDTH * GAME_HEIGHT];
} game_results_t;

#define GAME_RESULTS_FILEPATH "./results.db"

error_code game_results_load(Vector* results, const char* filepath);
error_code game_results_save(Vector* users, const char* filepath);

#endif
