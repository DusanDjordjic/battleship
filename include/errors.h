#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

// general errors
#define ERR_NONE  0000
// Error allocating memory
#define ERR_ALLOC 1001

// errors in io package
#define ERR_IO_IARG     2000
#define ERR_IO_UNKNOWN  2001
#define ERR_IO_IIN      2002

// Passwords are not matching
#define ERR_PASSWORDS_NO_MATCH 2003

// Invalid menu option selected
#define ERR_MENU_IOPTION 3000 

// Invalid port format in arguments
#define ERR_ARG_PORT_IFORMAT    4000
// Invalid port number 
#define ERR_ARG_IPORT           4001
// Invalid too few arguments passed
#define ERR_ARG_NOT_ENOUGH      4002

typedef uint16_t error_code;

#endif
