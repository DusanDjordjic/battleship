#include "errors.h"
#include "io.h"
#include "menu.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNREACHABLE                                                 \
	{                                                               \
		fprintf(stderr, "UNREACHABLE %s %d\n", __FILE__, __LINE__); \
		exit(1);                                                    \
	}

#define PORT_BASE 10
#define PORT_MAX ((1 << 16) - 1)

#define API_KEY_LEN 16

#define USERNAME_MAX_LEN 32
#define PASSWORD_MAX_LEN 32

typedef struct {
	uint32_t id;
	char username[USERNAME_MAX_LEN];
	char password[PASSWORD_MAX_LEN];
} user_t;

typedef struct {
	int sock_fd;
	struct sockaddr_in addr;
	user_t user;
	char api_token[API_KEY_LEN];
} client_state_t;

error_code client_signup(client_state_t* state);
error_code client_login(client_state_t* state);
error_code client_logout(client_state_t* state);
error_code connect_to_server(client_state_t* state);
error_code client_menu_create(menu_t* menu);

error_code parse_args(client_state_t* state, int argc, char** argv);
error_code parse_port(char* s_port, uint16_t* port);
void usage(char* exe);

void menu_item_display(menu_item_t* item)
{
	printf("%d) %s\n", item->index, item->prompt);
}

int main(int argc, char** argv)
{
	// IMPORTANT: initialize state to all zeros
	client_state_t state = { 0 };

	error_code err = parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// parse_args will log the error
		return 1;
	}

	err = connect_to_server(&state);
	if (err != ERR_NONE) {
		fprintf(stderr, "failed to connect to server: error code %d\n", err);
		return 1;
	}

	menu_t menu;
	err = client_menu_create(&menu);
	if (err != ERR_NONE) {
		fprintf(stderr, "failed to initalize menu: error code %d\n", err);
		return 1;
	}

	uint32_t choice;
	while (1) {

		error_code err = menu_display(&menu, &choice, menu_item_display);
		switch (err) {
		case ERR_MENU_IOPTION:
			fprintf(stderr, "invalid option selected\n");
			continue;
		case ERR_IO_IIN:
		case ERR_IO_IARG:
			fprintf(stderr, "invalid input\n");
			continue;
		case ERR_IO_UNKNOWN:
			fprintf(stderr, "unknown error, try again...");
			continue;
		case ERR_ALLOC:
			fprintf(stderr, "unrecoverable state, closing...");
			goto EXIT;
		case ERR_NONE:
			break;
		default:
			UNREACHABLE;
		}

		// Handle user input based on current page index and selected index
		switch (menu.current_index) {
		case 0:
			switch (choice) {
			case 1:
				err = client_signup(&state);
				if (err == ERR_NONE) {
					menu_set_page(&menu, 1);
				}
				break;
			case 2:
				err = client_login(&state);
				if (err == ERR_NONE) {
					menu_set_page(&menu, 1);
				}

				break;
			default:
				fprintf(stderr, "invalid choice\n");
				break;
			case 0:
				goto EXIT;
			}
			break;

		case 1:
			switch (choice) {
			case 1:
				printf("DO LIST OTHER PLAYERS\n");
				break;
			case 2:
				printf("DO CHALLENGE PLAYER\n");
				break;
			case 3:
				printf("DO PROFILE SETTINGS\n");
				break;
			default:
				fprintf(stderr, "invalid choice\n");
				break;
			case 0:
				err = client_logout(&state);
				if (err == ERR_NONE) {
					menu_set_page(&menu, 0);
				} else {
					// we failed to logout but we removed any state we had about user
					// so we are just going to close the app with an error
					fprintf(stderr, "we failed to logout and reached unrecoverable "
									"state, closing...\n");
					goto EXIT;
				}
				break;
			}
			break;
		default:
			UNREACHABLE;
		}
	}

EXIT:
	menu_deinit(&menu);

	return 0;
}

error_code connect_to_server(client_state_t* state)
{
	state->sock_fd = 0;
	// TODO connect to server
	return ERR_NONE;
}

error_code client_login(client_state_t* state)
{
	if (state->user.id != 0) {
		UNREACHABLE;
	}

	printf("Login process started\n");
	printf("Enter username (max %d): ", USERNAME_MAX_LEN);
	error_code err = read_line(state->user.username, USERNAME_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IO_IIN:
		fprintf(stderr, "invalid input\n");
		return err;
	case ERR_IO_UNKNOWN:
		fprintf(stderr, "unknown error: %s", strerror(errno));
		return err;
	case ERR_IO_IARG:
		// unreachable because we passed USERNAME_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	printf("\nEnter password (max %d): ", PASSWORD_MAX_LEN);
	err = read_line(state->user.password, PASSWORD_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IO_IIN:
		fprintf(stderr, "invalid input\n");
		return err;
	case ERR_IO_UNKNOWN:
		fprintf(stderr, "unknown error: %s", strerror(errno));
		return err;
	case ERR_IO_IARG:
		// unreachable because we passed PASSWORD_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	// TODO send login message to server and get the user_id back
	// update user with user_id
	state->user.id = 1;

	// Also login will return the api key as well
	strncpy(state->api_token, "API_T_FROM_SRV", API_KEY_LEN);
	return ERR_NONE;
}
error_code client_logout(client_state_t* state)
{
	if (state->user.id == 0) {
		UNREACHABLE;
	}

	// TODO Send logout message

	// Reset info we have about user and api key
	memset(&state->user, 0, sizeof(user_t));
	memset(state->api_token, 0, API_KEY_LEN);
	return ERR_NONE;
}

error_code client_signup(client_state_t* state)
{
	if (state->user.id != 0) {
		UNREACHABLE;
	}

	printf("Signup process started\n");
	printf("Enter username (max %d): ", USERNAME_MAX_LEN);
	error_code err = read_line(state->user.username, USERNAME_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IO_IIN:
		fprintf(stderr, "invalid input\n");
		return err;
	case ERR_IO_UNKNOWN:
		fprintf(stderr, "unknown error: %s", strerror(errno));
		return err;
	case ERR_IO_IARG:
		// unreachable because we passed USERNAME_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	printf("\nEnter password (max %d): ", PASSWORD_MAX_LEN);
	err = read_line(state->user.password, PASSWORD_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IO_IIN:
		fprintf(stderr, "invalid input\n");
		return err;
		UNREACHABLE;
	case ERR_IO_UNKNOWN:
		fprintf(stderr, "unknown error: %s", strerror(errno));
		return err;
	case ERR_IO_IARG:
		// unreachable because we passed PASSWORD_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	// TODO send signup message to server and get the user_id back
	// update user with user_id
	state->user.id = 1;

	// Also signup will do the login as well and we will get the token back
	strncpy(state->api_token, "API_T_FROM_SRV", API_KEY_LEN);
	return ERR_NONE;
}

error_code parse_args(client_state_t* state, int argc, char** argv)
{
	if (argc < 3) {
		usage(argv[0]);
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

void usage(char* exe) { printf("Usage: %s <server_ip> <server_port>\n", exe); }

error_code client_menu_create(menu_t* menu)
{
	error_code err = menu_init(menu, 2);
	if (err != ERR_NONE) {
		return err;
	}

	{
		menu_page_t page;
		err = menu_page_init(&page, 3);
		if (err != ERR_NONE) {
			return err;
		}

		{
			menu_item_t item = { .index = 1, .prompt = "Signup" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 2, .prompt = "Login" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 0, .prompt = "Quit" };
			menu_page_add_item(&page, item);
		}

		menu_add_page(menu, page);
	}

	{
		menu_page_t page;
		err = menu_page_init(&page, 4);
		if (err != ERR_NONE) {
			return err;
		}

		{
			menu_item_t item = { .index = 1, .prompt = "List other players" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 2, .prompt = "Challenge other player" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 3, .prompt = "Profile settings" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 0, .prompt = "Logout" };
			menu_page_add_item(&page, item);
		}
		menu_add_page(menu, page);
	}

	return ERR_NONE;
}
