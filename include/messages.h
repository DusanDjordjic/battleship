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

#endif
