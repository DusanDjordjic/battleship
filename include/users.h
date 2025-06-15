#ifndef USERS_H
#define USERS_H

#include "include/errors.h"
#include "include/globals.h"
#include "include/vector/vector.h"

typedef struct {
	char username[USERNAME_MAX_LEN];
	char password[PASSWORD_MAX_LEN];
	char repeatedPassword[PASSWORD_MAX_LEN];
} client_user_t;

typedef struct {
	char username[USERNAME_MAX_LEN];
	char password[PASSWORD_MAX_LEN];
} server_user_t;

// These are the functions used by server to store and load users 
// These functions work with server_user_t
error_code users_load(Vector* users, const char* filepath);
error_code users_save(Vector* users, const char* filepath);

#endif
