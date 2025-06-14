#ifndef GLOBALS_H
#define GLOBALS_H

#define PORT_BASE 10
#define PORT_MAX ((1 << 16) - 1)

#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32

#define API_KEY_LEN 33 // 32 + 1
#define ERROR_MESSAGE_MAX_LEN 129 // 128 + 1

// Message types 
#define MSG_SIGNUP 1
#define MSG_LOGIN 2

// Request was processed successfully
#define STATUS_OK 1
// There was a conflict (username already exists, field is already destroyed, etc..)
#define STATUS_CONFLICT 2
// Requested resource wasn't found
#define STATUS_NOT_FOUND 3
// Provided token is invalid
#define STATUS_UNAUTHORIZED 4

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

#endif
