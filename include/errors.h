#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

// general errors
#define ERR_NONE  0000
#define ERR_ALLOC 1001

// errors in io package
#define ERR_IO_IARG     2000
#define ERR_IO_UNKNOWN  2001
#define ERR_IO_IIN      2002


// errors in menu package
#define ERR_MENU_IOPTION 3000

#define ERR_ARG_PORT_IFORMAT    4000
#define ERR_ARG_IPORT           4001
#define ERR_ARG_NOT_ENOUGH      4002

typedef uint16_t error_code;

#endif
