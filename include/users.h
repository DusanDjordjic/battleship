#ifndef USERS_H
#define USERS_H

#include "include/errors.h"
#include "include/vector/vector.h"

error_code users_load(Vector* users, const char* filepath);
error_code users_save(Vector* users, const char* filepath);

#endif
