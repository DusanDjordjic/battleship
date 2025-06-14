#ifndef IO_H
#define IO_H

#include <stdint.h>
#include "errors.h"

char* error_to_string(error_code error);
void error_print(error_code error);

error_code read_line(char* line, uint32_t len);
error_code read_line_no_echo(char* line, uint32_t len);
error_code read_uint32(uint32_t* out);
error_code read_int32(int32_t* out);


#endif
