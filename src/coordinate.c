#include "include/globals.h"
#include "include/coordinate.h"

error_code coordinate_validate(Coordinate c) {
    if (c.x < 0) {
        fprintf(stderr,RED  "Invalid input for first coordinate %c\n" RESET, c.x + 'A');
        return ERR_ICOORDINATE;
    }

    if (c.x >= GAME_WIDTH) {
        fprintf(stderr,RED  "Invalid input for first coordinate %c\n" RESET, c.x + 'A');
        return ERR_ICOORDINATE;
    }

    if (c.y < 0) {
        fprintf(stderr,RED  "Invalid input for second coordinate %c\n" RESET, c.y + '1');
        return ERR_ICOORDINATE;
    }

    if (c.y >= GAME_HEIGHT) {
        fprintf(stderr,RED  "Invalid input for second coordinate %c\n" RESET, c.y + '1');
        return ERR_ICOORDINATE;
    }

    return ERR_NONE;
}
