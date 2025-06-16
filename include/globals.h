#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdlib.h>

#define UNREACHABLE                                                             \
	{                                                                           \
		fprintf(stderr, RED "UNREACHABLE %s %d\n" RESET, __FILE__, __LINE__);   \
		exit(1);                                                                \
	}

#define PORT_BASE 10
#define PORT_MAX ((1 << 16) - 1)

#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32

#define API_KEY_LEN 33 // 32 + 1
#define ERROR_MESSAGE_MAX_LEN 129 // 128 + 1

// Message types 
#define MSG_SIGNUP 1
#define MSG_LOGIN 2
#define MSG_LOGOUT 3
#define MSG_LIST_USERS 4
#define MSG_LOOK_FOR_GAME 5
#define MSG_CANCEL_LOOK_FOR_GAME 6
#define MSG_CHALLENGE_PLAYER 7

// Request was processed successfully
#define STATUS_OK 1
// There was a conflict (username already exists, field is already destroyed, etc..)
#define STATUS_CONFLICT 2
// Requested resource wasn't found
#define STATUS_NOT_FOUND 3
// Provided token is invalid
#define STATUS_UNAUTHORIZED 4
// Client sent a bad request (login while already logged in, etc...)
#define STATUS_BAD_REQUEST 5
// Client sent a bad request (login while already logged in, etc...)
#define STATUS_PLAYER_IS_NOT_CONNECTED 6
// Client sent a bad request (login while already logged in, etc...)
#define STATUS_PLAYER_IS_NOT_LOOKING_FOR_GAME 7

// Unknown error
#define STATUS_UNKNOWN_ERROR 255 


// Server input buffer size
#define IN_BUFFER_SIZE 1024

// Color codes
#define RED   "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE  "\033[34m"
#define RESET "\033[0m"


// Client flags 
#define CLIENT_LOGGED_IN (1 << 0)
#define CLIENT_LOOKING_FOR_GAME (1 << 1)

#endif
