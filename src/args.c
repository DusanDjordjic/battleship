#include <arpa/inet.h>
#include <assert.h>
#include <include/args.h>
#include <stdio.h>
#include <stdlib.h>

void client_usage(char* exe) { printf("Usage: %s <server_ip> <server_port>\n", exe); }
void server_usage(char* exe) { printf("Usage: %s <server_port>\n", exe); }

error_code client_parse_args(client_state_t* state, int argc, char** argv)
{
	if (argc < 3) {
		client_usage(argv[0]);
		return ERR_ARG_NOT_ENOUGH;
	}

	char* sip = argv[1];
	char* sport = argv[2];

	uint16_t port;
	error_code err = parse_port(sport, &port);
	if (err != ERR_NONE) {
		return err;
	}

	state->addr.sin_addr.s_addr = inet_addr(sip);
	state->addr.sin_family = AF_INET;
	state->addr.sin_port = htons(port);

	state->sock_fd = 0;
	return ERR_NONE;
}

error_code server_parse_args(server_state_t* state, int argc, char** argv)
{
	if (argc < 2) {
		server_usage(argv[0]);
		return ERR_ARG_NOT_ENOUGH;
	}

	char* sport = argv[1];

	uint16_t port;
	error_code err = parse_port(sport, &port);
	if (err != ERR_NONE) {
		return err;
	}

	state->port = port;
	return ERR_NONE;
}

error_code parse_port(char* s_port, uint16_t* port)
{
	assert(s_port != NULL);
	assert(*s_port != '\0');
	assert(port != NULL);

	char* rest = NULL;
	uint32_t parsed = strtoul(s_port, &rest, PORT_BASE);
	if (rest == NULL || *rest != '\0') {
		fprintf(stderr, "port \"%s\" is in invalid format\n", s_port);
		return ERR_ARG_PORT_IFORMAT;
	}

	if (parsed > PORT_MAX) {
		fprintf(stderr, "port number is invalid, max = %u : port %u\n", PORT_MAX,
			parsed);
		return ERR_ARG_IPORT;
	}

	*port = parsed;

	return ERR_NONE;
}
