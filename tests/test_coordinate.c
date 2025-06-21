#include "include/coordinate.h"
#include "include/globals.h"
#include <include/criterion/criterion.h>
#include <include/criterion/logging.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <include/server.h>
#include <include/errors.h>

#define TEST_PORT 8080

Test(coordinates, coordinate_validate) {
    typedef struct {
        Coordinate coordinate;
        error_code err;
    } TestCase;

    TestCase cases[] = {
        { .coordinate = { .x = -1, .y = 0 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = -1, .y = 1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = -1, .y = GAME_HEIGHT - 1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = GAME_WIDTH, .y =  0 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = GAME_WIDTH, .y =  1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = GAME_WIDTH, .y =  GAME_HEIGHT - 1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = 0, .y = -1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = 1, .y = -1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = GAME_WIDTH - 1, .y = -1 }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = 0, .y =  GAME_HEIGHT }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = 1, .y =  GAME_HEIGHT }, .err = ERR_ICOORDINATE },
        { .coordinate = { .x = GAME_WIDTH - 1, .y =  GAME_HEIGHT }, .err = ERR_ICOORDINATE },
    };

    int len = sizeof(cases) / sizeof(TestCase);

    // test cases that should fail
    for (int i = 0; i < len; i++) {
        TestCase tc = cases[i];
        error_code err = coordinate_validate(tc.coordinate);
        cr_assert_eq(err, tc.err, "Case %d: invalid error", i);
    }

    // should be valid
    for (int x = 0; x < GAME_WIDTH; x++) {
        for (int y = 0; y < GAME_HEIGHT; y++) {
            Coordinate c = { .x = x, .y = y };
            error_code err = coordinate_validate(c);
            cr_assert_eq(err, ERR_NONE, "%d %d expected to be valid", x, y);

        }
    }
}

