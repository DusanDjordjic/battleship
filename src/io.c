#include <include/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define NUMBER_LEN 32
#define NUMBER_BASE 10

char* error_to_string(error_code error)
{
	switch (error) {
    case ERR_NONE:
        return "ERROR: None";
    case ERR_ALLOC:
        return "ERROR: Allocation failed";
    case ERR_PASSWORDS_NO_MATCH:
        return "ERROR: Passwords don't match";
    case ERR_MENU_IOPTION:
        return "ERROR: Invalid menu option selected";
    case ERR_ARG_PORT_IFORMAT:
        return "ERROR: Port number is in wrong format";
    case ERR_ARG_IPORT:
        return "ERROR: Port number is invalid";
    case ERR_ARG_NOT_ENOUGH:
        return "ERROR: Not enough arguments";
	case ERR_IO_IIN:
		return "ERROR: Invalid input";
	case ERR_IO_IARG:
		return "ERROR: Invalid parameters/arguments";
	case ERR_IO_UNKNOWN:
		return "ERROR: Unknown error";
	default:
		return "UNREACHABLE";
	}
}

void error_print(error_code error)
{
	fprintf(stderr, "%s\n", error_to_string(error));
}

static void clear_stdin()
{
	int tmp;
	while ((tmp = getc(stdin)) != '\n' && tmp != EOF) { }
}

static uint32_t string_find(char target, const char* str, uint32_t len)
{
	for (uint32_t i = 0; i < len; i++) {
		if (str[i] == target) {
			return i;
		}
	}

	return 0;
}

error_code read_line(char* line, uint32_t len)
{
	// We need 1 char for '\0' so if len < 2
	// we return an error because
	// there is no space for other chars
	if (len < 2) {
		return ERR_IO_IARG;
	}

	char* res = fgets(line, (int32_t)len, stdin);
	if (res == NULL) {
		return ERR_IO_UNKNOWN;
	}

	// check if we have '\n' in line.
	// if we do there is no need to clear stdin
	// but we have to replace it with \0
	if (*line == '\n') {
		// User just pressed enter without typing any chars
		return ERR_IO_IIN;
	}

	uint32_t index = string_find('\n', line, len);
	if (index != 0) {
		line[index] = '\0';
	} else {
		clear_stdin();
	}

	return ERR_NONE;
}

error_code read_line_no_echo(char* line, uint32_t len)
{
	// We need 1 char for '\0' so if len < 2
	// we return an error because
	// there is no space for other chars
	if (len < 2) {
		return ERR_IO_IARG;
	}

    struct termios old;
    struct termios new;
    
    tcgetattr(STDIN_FILENO, &old);
    new = old;

    // Delete echo flag
    new.c_lflag &= ~(ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new);

	char* res = fgets(line, (int32_t)len, stdin);

    // Restore all configuration
    tcsetattr(STDIN_FILENO, TCSANOW, &old);

	if (res == NULL) {
		return ERR_IO_UNKNOWN;
	}

	// check if we have '\n' in line.
	// if we do there is no need to clear stdin
	// but we have to replace it with \0
	if (*line == '\n') {
		// User just pressed enter without typing any chars
		return ERR_IO_IIN;
	}

	uint32_t index = string_find('\n', line, len);
	if (index != 0) {
		line[index] = '\0';
	} else {
		clear_stdin();
	}

	return ERR_NONE;
}

error_code read_uint32(uint32_t* out)
{
	char line[NUMBER_LEN];

	int res = read_line(line, NUMBER_LEN);
	if (res != ERR_NONE) {
		return res;
	}

	char* rest;
	uint32_t num = strtoul(line, &rest, NUMBER_BASE);
	if (*rest == '\0') {
		// whole string is valid
		*out = num;
		return ERR_NONE;
	}

	if (rest != line) {
		// part of string is valid and we parsed something
		*out = num;
		return ERR_NONE;
	}

	// nothing in string is valid number
	return ERR_IO_IIN;
}

error_code read_int32(int32_t* out)
{
	*out = 0;
	return ERR_NONE;
}
