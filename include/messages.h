#ifndef MESSAGES_H
#define MESSAGES_H

#include "include/errors.h"
#include <stdint.h>
#include <include/globals.h>

error_code send_message(int sock_fd, const void* message, uint32_t len);
error_code read_message(int sock_fd, void* buffer, uint32_t len);

// Responses
typedef struct {
    uint8_t status_code;
    char message [ERROR_MESSAGE_MAX_LEN];
} ErrorResponseMessage;

typedef struct {
    uint8_t status_code;
    char api_key[API_KEY_LEN];
} SignupSuccessResponseMessage;

typedef union {
    SignupSuccessResponseMessage success;
    ErrorResponseMessage error; 
} SignupResponseMessage;

typedef struct {
    uint8_t status_code;
    char api_key[API_KEY_LEN];
} LoginSuccessResponseMessage;

typedef union {
    LoginSuccessResponseMessage success;
    ErrorResponseMessage error; 
} LoginResponseMessage;

typedef struct {
    uint8_t status_code;
} LogoutSuccessResponseMessage;

typedef union {
    LogoutSuccessResponseMessage success;
    ErrorResponseMessage error; 
} LogoutResponseMessage;

typedef struct {
    uint8_t status_code;
    uint32_t count;
} ListUsersSuccessResponseMessage;

typedef union {
    ListUsersSuccessResponseMessage success;
    ErrorResponseMessage error; 
} ListUsersResponseMessage;

typedef struct {
    uint8_t status_code;
} LookForGameSuccessResponseMessage;

typedef union {
    LookForGameSuccessResponseMessage success;
    ErrorResponseMessage error; 
} LookForGameResponseMessage;

typedef struct {
    uint8_t status_code;
} CancelLookForGameSuccessResponseMessage;

typedef union {
    CancelLookForGameSuccessResponseMessage success;
    ErrorResponseMessage error; 
} CancelLookForGameResponseMessage;

typedef struct {
    uint8_t status_code;
    uint32_t lobby_id;
} LobbyIDResponse;

typedef union {
    LobbyIDResponse success;
    ErrorResponseMessage error; 
} ChallengePlayerResponseMessage;

// Requests 
typedef struct {
    uint8_t type;
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
} SignupRequestMessage;

typedef struct {
    uint8_t type;
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
} LoginRequestMessage;

typedef struct {
    uint8_t type;
    char api_key[API_KEY_LEN];
} LogoutRequestMessage;

typedef struct {
    uint8_t type;
    char api_key[API_KEY_LEN];
} ListUsersRequestMessage;

typedef struct {
    uint8_t type;
    char api_key[API_KEY_LEN];
} LookForGameRequestMessage;

typedef struct {
    uint8_t type;
    char api_key[API_KEY_LEN];
} CancelLookForGameRequestMessage;

// Set by the client to challenge other clients
// target_username is the username 
// of the client you wish to play with
typedef struct {
    uint8_t type;
    char api_key[API_KEY_LEN];
    char target_username[USERNAME_MAX_LEN];
} ChallengePlayerRequestMessage;

// Sent by the server to the client that 
// is being challenged to a game
typedef struct {
    uint8_t type;
    char challenger_username[USERNAME_MAX_LEN];
} ServerEventAcceptChallengeRequestMessage;

// sent by the client to say if he accepts the game or not
typedef struct {
    uint8_t type;
    uint8_t accept;
} ClientEventAcceptChallengeRequestMessage;

#endif
