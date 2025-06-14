#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>
#include <include/globals.h>

typedef struct {
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
} SignupRequestMessage;

typedef struct {
    uint8_t status_code;
    char token[API_KEY_LEN];
} SignupSuccessResponseMessage;

typedef struct {
    uint8_t status_code;
    char message [ERROR_MESSAGE_MAX_LEN];
} SignupErrorResponseMessage;

typedef union {
    SignupSuccessResponseMessage success;
    SignupErrorResponseMessage error; 
} SignupResponseMessage;

#endif
