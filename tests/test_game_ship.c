#include "include/errors.h"
#include "include/game_ship.h"
#include <include/criterion/criterion.h>
#include <include/criterion/logging.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <include/server.h>

Test(ships, ship_validate_coordinates) {
    typedef struct {
        GameShip ship;
        error_code err;
    } TestCase;

    TestCase cases[] = {
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 1 }, .end = { .x = 0, .y = 1 } }, .err = ERR_NONE },
        { .ship = { .width = 1, .height = 1, .start = { .x = 1, .y = 0 }, .end = { .x = 1, .y = 0 } }, .err = ERR_NONE },

        { .ship = { .width = 1, .height = 1, .start = { .x = 1, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 1 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 1, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 1 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 0, .y = 1 }, .end = { .x = 1, .y = 1 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 1, .y = 0 }, .end = { .x = 1, .y = 1 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 1, .y = 1 }, .end = { .x = 0, .y = 1 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 1, .height = 1, .start = { .x = 1, .y = 1 }, .end = { .x = 1, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },

        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 1, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 1 } }, .err = ERR_NONE },
        { .ship = { .width = 2, .height = 1, .start = { .x = 1, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 1 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 2, .height = 1, .start = { .x = 1, .y = 2 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .width = 2, .height = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 2, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 2, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 2, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 2, .y = 2 }, .end = { .x = 0, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 2, .height = 1, .start = { .x = 2, .y = 2 }, .end = { .x = 2, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },

        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 2, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 2 } }, .err = ERR_NONE },
        { .ship = { .width = 3, .height = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 3, .height = 1, .start = { .x = 1, .y = 3 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .width = 3, .height = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 3, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 3, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 3, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 3, .y = 3 }, .end = { .x = 0, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 3, .height = 1, .start = { .x = 3, .y = 3 }, .end = { .x = 3, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },

        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 3, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 3 } }, .err = ERR_NONE },
        { .ship = { .width = 4, .height = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .width = 4, .height = 1, .start = { .x = 1, .y = 4 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .width = 4, .height = 1, .start = { .x = 4, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 4 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 4, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 0, .y = 4 }, .end = { .x = 4, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 4, .y = 0 }, .end = { .x = 4, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 4, .y = 4 }, .end = { .x = 0, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .width = 4, .height = 1, .start = { .x = 4, .y = 4 }, .end = { .x = 4, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        
        // SWAP WIDTH AND HEIGHT
        
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 1, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 1 } }, .err = ERR_NONE },
        { .ship = { .height = 2, .width = 1, .start = { .x = 1, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 1 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 2, .width = 1, .start = { .x = 1, .y = 2 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .height = 2, .width = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 2, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 2, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 2, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 2, .y = 2 }, .end = { .x = 0, .y = 2 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 2, .width = 1, .start = { .x = 2, .y = 2 }, .end = { .x = 2, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },

        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 2, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 2 } }, .err = ERR_NONE },
        { .ship = { .height = 3, .width = 1, .start = { .x = 2, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 2 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 3, .width = 1, .start = { .x = 1, .y = 3 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .height = 3, .width = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 3, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 3, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 3, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 3, .y = 3 }, .end = { .x = 0, .y = 3 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 3, .width = 1, .start = { .x = 3, .y = 3 }, .end = { .x = 3, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },

        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 3, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 3 } }, .err = ERR_NONE },
        { .ship = { .height = 4, .width = 1, .start = { .x = 3, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 3 }, .end = { .x = 0, .y = 0 } }, .err = ERR_NONE },
        { .ship = { .height = 4, .width = 1, .start = { .x = 1, .y = 4 }, .end = { .x = 1, .y = 1 } }, .err = ERR_NONE },

        { .ship = { .height = 4, .width = 1, .start = { .x = 4, .y = 0 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 4 }, .end = { .x = 0, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 4, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 0 }, .end = { .x = 0, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 0, .y = 4 }, .end = { .x = 4, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 4, .y = 0 }, .end = { .x = 4, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 4, .y = 4 }, .end = { .x = 0, .y = 4 } }, .err = ERR_SHIP_ICOORDINATES },
        { .ship = { .height = 4, .width = 1, .start = { .x = 4, .y = 4 }, .end = { .x = 4, .y = 0 } }, .err = ERR_SHIP_ICOORDINATES },
    };

    int len = sizeof(cases) / sizeof(TestCase);

    for (int i = 0; i < len; i++) {
        TestCase c = cases[i];
        error_code err = game_ship_validate_coordinates(c.ship);
        cr_assert_eq(err, c.err, "%d invalid error", i);
    }
}
