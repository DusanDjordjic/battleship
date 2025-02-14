#ifndef ARGS_H
#define ARGS_H

#include <include/errors.h>
#include <include/state.h>

#define PORT_BASE 10
#define PORT_MAX ((1 << 16) - 1)

error_code client_parse_args(client_state_t* state, int argc, char** argv);
error_code server_parse_args(server_state_t* state, int argc, char** argv);
error_code parse_port(char* s_port, uint16_t* port);

#endif
