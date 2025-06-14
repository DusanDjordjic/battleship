#ifndef MESSAGES_H
#define MESSAGES_H

#include "include/errors.h"
#include <stdint.h>
#include <include/globals.h>

typedef struct {
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
} SignupRequestMessage;

typedef struct {
    uint8_t status_code;
    uint32_t user_id;
    char api_key[API_KEY_LEN];
} SignupSuccessResponseMessage;

typedef struct {
    uint8_t status_code;
    char message [ERROR_MESSAGE_MAX_LEN];
} SignupErrorResponseMessage;

typedef union {
    SignupSuccessResponseMessage success;
    SignupErrorResponseMessage error; 
} SignupResponseMessage;

error_code send_message(int sock_fd, const void* message, uint32_t len);
error_code read_message(int sock_fd, void* buffer, uint32_t len);

#endif
