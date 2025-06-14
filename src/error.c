#include <include/errors.h>
#include <stdio.h>

char* error_to_string(error_code error)
{
	switch (error) {
    case ERR_ALLOC:
        return "ERROR: Allocation failed";
    case ERR_SEND_NO_MEM:
        return "ERROR: Failed to send message due to running out of memory";
    case ERR_NONE:
        return "ERROR: None";
	case ERR_UNKNOWN:
		return "ERROR: Unknown error";
	case ERR_IARG:
		return "ERROR: Invalid parameters/arguments";
	case ERR_IIN:
		return "ERROR: Invalid input";
    case ERR_IFD:
		return "ERROR: Invalid socket/file descriptor";
    case ERR_ARG_IPORT:
        return "ERROR: Port number is invalid";
    case ERR_ARG_NOT_ENOUGH:
        return "ERROR: Not enough arguments";
    case ERR_ARG_PORT_IFORMAT:
        return "ERROR: Port number is in wrong format";
    case ERR_PASSWORDS_NO_MATCH:
        return "ERROR: Passwords don't match";
    case ERR_MENU_IOPTION:
        return "ERROR: Invalid menu option selected";
    case ERR_PEER_CLOSED:
        return "ERROR: Connection peer closed the connection";
	default:
		return "UNREACHABLE";
	}
}

void error_print(error_code error)
{
	fprintf(stderr, "%s\n", error_to_string(error));
}
