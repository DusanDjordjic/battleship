#include <include/globals.h>
#include <include/messages.h>
#include <asm-generic/errno-base.h>
#include <include/errors.h>
#include <include/io.h>
#include <include/menu.h>
#include <include/args.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <include/state.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define UNREACHABLE                                                             \
	{                                                                           \
		fprintf(stderr, RED "UNREACHABLE %s %d\n" RESET, __FILE__, __LINE__);   \
		exit(1);                                                                \
	}

error_code client_signup(client_state_t* state);
error_code client_login(client_state_t* state);
error_code client_logout(client_state_t* state);
error_code connect_to_server(client_state_t* state);
error_code client_menu_create(menu_t* menu);

void menu_item_display(menu_item_t* item)
{
	fprintf(stdout, "%d) %s\n", item->index, item->prompt);
}

int main(int argc, char** argv)
{
	// IMPORTANT: initialize state to all zeros
	client_state_t state = { 0 };

	error_code err = client_parse_args(&state, argc, argv);
	if (err != ERR_NONE) {
		// parse_args will log the error
		return 1;
	}

	err = connect_to_server(&state);
	if (err != ERR_NONE) {
		fprintf(stderr, RED "ERROR: Failed to connect to server: error code %d\n" RESET, err);
		return 1;
	}

	menu_t menu;
	err = client_menu_create(&menu);
	if (err != ERR_NONE) {
		fprintf(stderr, RED "ERROR: failed to initalize menu: error code %d\n" RESET, err);
		return 1;
	}

	uint32_t choice;
	while (1) {

		error_code err = menu_display(&menu, &choice, menu_item_display);
		switch (err) {
		case ERR_MENU_IOPTION:
		case ERR_IIN:
		case ERR_IARG:
		case ERR_UNKNOWN:
            error_print(err);
			continue;
		case ERR_ALLOC:
            error_print(err);
			fprintf(stderr, RED "ERROR: Unrecoverable state, closing..." RESET);
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
				fprintf(stderr, RED "ERROR: Invalid choice\n" RESET);
				break;
			case 0:
				goto EXIT;
			}
			break;

		case 1:
			switch (choice) {
			case 1:
				printf(BLUE "DO LIST OTHER PLAYERS\n" RESET);
				break;
			case 2:
				printf(BLUE "DO CHALLENGE PLAYER\n" RESET);
				break;
			case 3:
				printf(BLUE "DO PROFILE SETTINGS\n" RESET);
				break;
			default:
				fprintf(stderr, RED "ERROR: nvalid choice\n" RESET);
				break;
			case 0:
				err = client_logout(&state);
				if (err == ERR_NONE) {
					menu_set_page(&menu, 0);
				} else {
					// we failed to logout but we removed any state we had about user
					// so we are just going to close the app with an error
					fprintf(stderr, RED "we failed to logout and reached unrecoverable "
									"state, closing...\n" RESET);
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
    if (state.sock_fd != 0) {
        close(state.sock_fd);
    }

	return 0;
}

error_code connect_to_server(client_state_t* state)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        switch (errno) {
            case EACCES:
                return ERR_PERMISSION_DENIED;
            default:
                return ERR_UNKNOWN;
        }
    }

    int ret = connect(sock_fd, (struct sockaddr*)&(state->addr), sizeof(struct sockaddr_in));
    if (ret == -1) {
        switch (errno) {
            case EACCES:
                return ERR_PERMISSION_DENIED;
            case ENETUNREACH:
                // Connection is unreachable because address is invalid
            case ECONNREFUSED:
                // Connection refused because address is invalid
            case EAFNOSUPPORT:
                // Passed address didn't have correct address family
                return ERR_IARG;
            case EBADF:
                // Passed socket it not valid
                return ERR_IFD;
            default:
                return ERR_UNKNOWN;
        }
    }

    state->sock_fd = sock_fd;

	return ERR_NONE;
}

error_code client_login(client_state_t* state)
{
	if (state->logged_in) {
		UNREACHABLE;
	}

	fprintf(stdout, "Login process started\n");
	fprintf(stdout, "Enter username (max %d): ", USERNAME_MAX_LEN);
	error_code err = read_line(state->user.username, USERNAME_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(err);
		return err;
	case ERR_IARG:
		// unreachable because we passed USERNAME_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	fprintf(stdout, "Enter password (max %d) (HIDDEN): ", PASSWORD_MAX_LEN);
	err = read_line_no_echo(state->user.password, PASSWORD_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(err);
		return err;
	case ERR_IARG:
		// unreachable because we passed PASSWORD_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	// TODO send login message to server and get the user_id back
	// update user with user_id
	state->logged_in = 1;

	// Also login will return the api key as well
	strncpy(state->api_token, "API_T_FROM_SRV", API_KEY_LEN);
	return ERR_NONE;
}

error_code client_logout(client_state_t* state)
{
	if (!state->logged_in) {
		UNREACHABLE;
	}

	// TODO Send logout message

    state->logged_in = 0;
	// Reset info we have about user and api key
	memset(&state->user, 0, sizeof(client_user_t));
	memset(state->api_token, 0, API_KEY_LEN);
	return ERR_NONE;
}

error_code client_signup(client_state_t* state)
{
	if (state->logged_in) {
		UNREACHABLE;
	}

	fprintf(stdout, "Signup process started\n");
	fprintf(stdout, "Enter username (max %d): ", USERNAME_MAX_LEN);
	error_code err = read_line(state->user.username, USERNAME_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(err);
		return err;
	case ERR_IARG:
		// unreachable because we passed USERNAME_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	fprintf(stdout, "Enter password (max %d) (HIDDEN): ", PASSWORD_MAX_LEN);
	err = read_line_no_echo(state->user.password, PASSWORD_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(err);
		return err;
	case ERR_IARG:
		// unreachable because we passed PASSWORD_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

	fprintf(stdout, "\nRepeat password (max %d) (HIDDEN): ", PASSWORD_MAX_LEN);
	err = read_line_no_echo(state->user.repeatedPassword, PASSWORD_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(ERR_UNKNOWN);
		return err;
	case ERR_IARG:
		// unreachable because we passed PASSWORD_MAX_LEN as string len
		UNREACHABLE;
	default:
		UNREACHABLE;
	}

    // Print new line because after user repeats password 
    // next text was on the same line
    fprintf(stdout, "\n");

    int cmp = strncmp(state->user.password, state->user.repeatedPassword, PASSWORD_MAX_LEN);
    if (cmp != 0) {
        error_print(ERR_PASSWORDS_NO_MATCH);
        return ERR_PASSWORDS_NO_MATCH;
    }


    SignupRequestMessage req; 
    req.type = MSG_SIGNUP;
    strncpy(req.username, state->user.username, USERNAME_MAX_LEN);
    strncpy(req.password, state->user.password, PASSWORD_MAX_LEN);
    
    err = send_message(state->sock_fd, &req, sizeof(SignupRequestMessage)); 
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to send message, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }

   
    SignupResponseMessage res = { 0 };
    err = read_message(state->sock_fd, &res, sizeof(SignupResponseMessage));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to read message, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }
   
    
    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: Signup request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_CONFLICT) {
            return ERR_USERNAME_EXISTS;
        }

        return ERR_UNKNOWN;
    }

    fprintf(stdout, GREEN "Signed up successfully\n" RESET);
    
	state->logged_in = 1;
	strncpy(state->api_token, res.success.api_key, API_KEY_LEN);
	return ERR_NONE;
}

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
