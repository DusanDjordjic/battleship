#include "include/errors.h"
#include "include/globals.h"
#include "include/game_ship.h"
#include <stdio.h>

error_code game_ship_validate_coordinates(GameShip ship) {
    // ship width and height need to match coordinates
    // first wigure out ship dimensions by looking and coordinates
    uint8_t height = 0;
    uint8_t width = 0;

    if (ship.start.x > ship.end.x) {
        width = ship.start.x - ship.end.x + 1;
    } else if (ship.start.x < ship.end.x) {
        width = ship.end.x - ship.start.x + 1;
    } else {
        width = 1;
    }

    if (ship.start.y > ship.end.y) {
        height = ship.start.y - ship.end.y + 1;
    } else if (ship.start.y < ship.end.y) {
        height = ship.end.y - ship.start.y + 1;
    } else {
        height = 1;
    }

    if (width == ship.width && height == ship.height) {
        return ERR_NONE;
    }

    // Is ship vertically oriented?
    if (width == ship.height && height == ship.width) {
        return ERR_NONE;
    }

    uint8_t swidth  = ship.width;
    uint8_t sheight = ship.height;

    // If ship is vertically oriented swap ship's width and height 
    // so errors are correctly printed
    if (height > width) {
        uint8_t temp = swidth;
        swidth = sheight;
        sheight = temp; 
    }

    if (width != swidth) {
        fprintf(stderr, RED "Ship width needs to be %d but looking at coordinates (%c,%c) (%c,%c) ship width is %d = |%c - %c| + 1\n" RESET, 
                swidth, ship.start.x + 'A', ship.start.y + '1', ship.end.x + 'A', ship.end.y + '1', width, ship.start.x + 'A', ship.end.x + 'A');
        return ERR_SHIP_ICOORDINATES;
    }

    if (height != sheight) {
        fprintf(stderr, RED "Ship height needs to be %d but looking at coordinates (%c,%c) (%c,%c) ship height is %d = |%c - %c| + 1\n" RESET, 
                sheight, ship.start.x + 'A', ship.start.y + '1', ship.end.x + 'A', ship.end.y + '1', height , ship.start.y + '1', ship.end.y + '1');
        return ERR_SHIP_ICOORDINATES;
    }

    UNREACHABLE;
}


// Validate that the ship can be placed on board. 
// Ships cannot be next to one another.
error_code game_ship_validate_fields(uint8_t* game_state, GameShip ship) {
    // Fields occupied by current ship
    Coordinate already_placed[ship.width * ship.height];
    uint8_t already_placed_count = 0;

    for (uint8_t x = ship.start.x; x <= ship.end.x; x++) {
        for (uint8_t y = ship.start.y; y <= ship.end.y; y++) {
            // These need to be empty
            // . X . 
            // X X X
            // . X .
            for (char i = x - 1; i <= x + 1; i++) {
                for (char j = y - 1; j <= y + 1; j++) {
                    // TOP LEFT
                    if (i == x - 1 && j == y - 1) {
                        continue;
                    }
                    // BOTTOM LEFT
                    if (i == x - 1 && j == y + 1) {
                        continue;
                    }
                    // TOP RIGHT
                    if (i == x + 1 && j == y - 1) {
                        continue;
                    }
                    // BOTTOM RIGHT
                    if (i == x + 1 && j == y + 1) {
                        continue;
                    }

                    // Outside of board
                    if (i < 0 || j < 0 || i >= GAME_WIDTH || j >= GAME_HEIGHT) {
                        continue;
                    }
                    
                    uint8_t skip = 0;

                    for (uint8_t k = 0; k < already_placed_count; k++) {
                        Coordinate c = already_placed[k];
                        if (c.x == i && c.y == j) {
                            skip = 1;
                            break;
                        }
                    }

                    if(skip) {
                        continue;
                    }

                    
                    uint8_t f = game_state[i + j * GAME_WIDTH]; 
                    if (f != GAME_FIELD_EMPTY) {
                        fprintf(stderr, RED "Cannot place ship at (%c,%c) because there is a ship at (%c,%c) and between ships needs to be an empty space\n" RESET,
                                x + 'A', y + '1', i + 'A', j + '1');
                        return ERR_SHIP_COORDINATE_OCCUPIED;
                    }
                }
            }

            already_placed[already_placed_count].x = x;
            already_placed[already_placed_count].y = y;
            already_placed_count++;
        }
    }

    return ERR_NONE;
}
