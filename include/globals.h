#ifndef GLOBALS_H
#define GLOBALS_H

#define PORT_BASE 10
#define PORT_MAX ((1 << 16) - 1)

#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32

#define API_KEY_LEN 16
#define ERROR_MESSAGE_MAX_LEN 128


// Request was processed successfully
#define STATUS_OK 0
// There was a conflict (username already exists, field is already destroyed, etc..)
#define STATUS_CONFLICT 1
// Requested resource wasn't found
#define STATUS_NOT_FOUND 2
// Provided token is invalid
#define STATUS_UNAUTHORIZED 3

// Unknown error
#define STATUS_UNKNOWN_ERROR 255 
#endif
