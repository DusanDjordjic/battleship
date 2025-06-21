#ifndef COORDINATE_H
#define COORDINATE_H

#include "include/errors.h"
#include <stdint.h>

typedef struct {
    int8_t x;
    int8_t y;
} Coordinate;

error_code coordinate_validate(Coordinate c);

#endif
