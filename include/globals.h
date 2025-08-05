#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdlib.h>
#include <stdio.h>

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
#define MSG_CHALLENGE_QUESTION 8 // This is sent to the client
#define MSG_CHALLENGE_ANSWER 9
#define MSG_GAME_START 10
// Player is sending the coordinate where he made a shot
#define MSG_PLAYERS_SHOT 11
// Informing the other player where the shot went
#define MSG_REGISTER_SHOT 12

// Request was processed successfully
#define STATUS_OK 1
// There was a conflict (username already exists, etc..)
#define STATUS_CONFLICT 2
// Requested resource wasn't found
#define STATUS_NOT_FOUND 3
// Provided token is invalid
#define STATUS_UNAUTHORIZED 4
// Client sent a bad request (login while already logged in, etc...)
#define STATUS_BAD_REQUEST 5
// Other player is not connected
#define STATUS_PLAYER_IS_NOT_CONNECTED 6
// Other player is not looking for a game
#define STATUS_PLAYER_IS_NOT_LOOKING_FOR_GAME 7
// Other player failed to respond successfully
#define STATUS_PLAYER_ERROR 8
// Other player declined game
#define STATUS_PLAYER_DECLINED 9
// Player sent a request related to game but he isn't playing any game
#define STATUS_GAME_NOT_STARTED 10
// Player closed the connection so the game is abandoned
#define STATUS_GAME_ABANDONED 11
// Player shot at invalid field (outside of the board)
#define STATUS_SHOT_INVALID_FIELD 12
// Player shot at already destoryed field
#define STATUS_SHOT_ALREADY_DESTROYED 13
// Player sent a shot but its not his turn
#define STATUS_GAME_NOT_MY_TURN 14

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

// Game specific flags
#define GAME_STATE_CLOSED 0
#define GAME_STATE_ACCEPTING 1
#define GAME_STATE_WAITING_FOR_PLAYERS_STATES 2
#define GAME_STATE_STARTED 3
#define GAME_STATE_FINISHED 4

#define GAME_WIDTH 3
#define GAME_HEIGHT 3

#define GAME_FIELD_EMPTY 0
#define GAME_FIELD_SHIP 1
#define GAME_FIELD_HIT 2
#define GAME_FIELD_MISS 3
#define GAME_FIELD_INVALID 255

// Is it first player's turn or second
#define GAME_FIRSTS_TURN 1
#define GAME_SECONDS_TURN 2

#define GAME_FIRST_WON 1
#define GAME_SECOND_WON 2
#endif
