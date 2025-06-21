#ifndef GAME_SHIP_H
#define GAME_SHIP_H
    
#include "include/coordinate.h"
#include "include/errors.h"

typedef struct {
    Coordinate start;
    Coordinate end;
    uint8_t width;
    uint8_t height;
} GameShip;

error_code game_ship_validate_coordinates(GameShip ship);
error_code game_ship_validate_fields(uint8_t* game_state, GameShip ship);

#endif
