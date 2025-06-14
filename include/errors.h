#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

typedef uint16_t error_code;

char* error_to_string(error_code error);
void error_print(error_code error);

// Errors that should lead to exiting the program
// Failed to allocate memory
#define ERR_ALLOC 1001
// Failed to send message due to running out of memory
#define ERR_SEND_NO_MEM 1002

// General Errors
// No Error
#define ERR_NONE  0000
// Unknown error
#define ERR_UNKNOWN 2000
// Invalid Argument
#define ERR_IARG 2001
// Invalid input
#define ERR_IIN 2002
// Invalid file descriptior/socket
#define ERR_IFD 2003
// Invalid port number
#define ERR_ARG_IPORT 2004
// Invalid too few arguments passed
#define ERR_ARG_NOT_ENOUGH 2005
// Invalid port format in arguments
#define ERR_ARG_PORT_IFORMAT    2006
// Passwords are not matching
#define ERR_PASSWORDS_NO_MATCH 2007
// Invalid menu option selected
#define ERR_MENU_IOPTION 2008
// Peer closed the connection
#define ERR_PEER_CLOSED 2009
// Permission denied 
#define ERR_PERMISSION_DENIED 2010

// Server Errors 
#define ERR_USERNAME_EXISTS 3001

#endif
