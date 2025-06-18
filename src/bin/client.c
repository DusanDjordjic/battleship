#include <pthread.h>
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>

error_code client_signup(client_state_t* state);
error_code client_login(client_state_t* state);
error_code client_logout(client_state_t* state);
error_code client_list_users(client_state_t* state);
error_code client_look_for_game(client_state_t* state);
error_code client_cancel_look_for_game(client_state_t* state);
error_code client_challenge_player(client_state_t* state);

error_code connect_to_server(client_state_t* state);
error_code client_menu_create(menu_t* menu);
void* print_loading(void* param);

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
                client_list_users(&state);
				break;
			case 2:
                client_look_for_game(&state);
				break;
			case 3:
                client_challenge_player(&state);
				break;
			case 4:
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

    // Print new line because after user repeats password 
    // next text was on the same line
    fprintf(stdout, "\n");

    LoginRequestMessage req; 
    req.type = MSG_LOGIN;
    strncpy(req.username, state->user.username, USERNAME_MAX_LEN);
    strncpy(req.password, state->user.password, PASSWORD_MAX_LEN);
    
    err = send_message(state->sock_fd, &req, sizeof(req)); 
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to send login request, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }

    LoginResponseMessage res = { 0 };
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to read login response, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }
   
    
    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: Login request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_CONFLICT) {
            return ERR_USERNAME_EXISTS;
        }

        return ERR_UNKNOWN;
    }

    fprintf(stdout, GREEN "Logged in successfully\n" RESET);
    
	state->logged_in = 1;
	strncpy(state->api_key, res.success.api_key, API_KEY_LEN);

	return ERR_NONE;
}

error_code client_logout(client_state_t* state)
{
	if (!state->logged_in) {
		UNREACHABLE;
	}

	fprintf(stdout, "Logout process started\n");

    LogoutRequestMessage req;
    req.type = MSG_LOGOUT;
    strncpy(req.api_key, state->api_key, API_KEY_LEN);

    error_code err = send_message(state->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to send logout request, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }

    LogoutResponseMessage res = { 0 };
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to read logout response, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }
   
    
    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: Logout request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_UNAUTHORIZED) {
            return ERR_UNATHORIZED;
        }

        return ERR_UNKNOWN;
    }

    fprintf(stdout, GREEN "Logged out successfully\n" RESET);

    state->logged_in = 0;
	// Reset info we have about user and api key
	memset(&state->user, 0, sizeof(client_user_t));
	memset(state->api_key, 0, API_KEY_LEN);
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
    
    err = send_message(state->sock_fd, &req, sizeof(req)); 
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to send signup request, %d - %s\n" RESET, err, error_to_string(err));
        return err;
    }

   
    SignupResponseMessage res = { 0 };
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: Failed to read signup response, %d - %s\n" RESET, err, error_to_string(err));
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
	strncpy(state->api_key, res.success.api_key, API_KEY_LEN);
	return ERR_NONE;
}

error_code client_list_users(client_state_t* state) {
    ListUsersRequestMessage req;
    req.type = MSG_LIST_USERS;
    strncpy(req.api_key, state->api_key, API_KEY_LEN);

    error_code err = send_message(state->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to send list users request\n" RESET, error_to_string(err));
        return err;
    }

    ListUsersResponseMessage res;
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to read list users response\n" RESET, error_to_string(err));
        return err;
    }

    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: List users request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_UNAUTHORIZED) {
            return ERR_UNATHORIZED;
        }

        return ERR_UNKNOWN;
    }


    char username[USERNAME_MAX_LEN];

    fprintf(stdout, "Listing %u users\n(Green looking for games):\n", res.success.count);
    fprintf(stdout, "==========================\n");

    uint8_t looking_for_game = 0;
    for (uint32_t i = 0; i < res.success.count; i++) {
        err = read_message(state->sock_fd, &looking_for_game, sizeof(looking_for_game));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s Failed to looking for game\n" RESET, error_to_string(err));
            return err;
        }

        err = read_message(state->sock_fd, username, USERNAME_MAX_LEN);
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s Failed to read username\n" RESET, error_to_string(err));
            return err;
        }

        if (looking_for_game) {
            fprintf(stdout, GREEN "\"%s\"\n" RESET, username);
        } else {
            fprintf(stdout, "\"%s\"\n", username);
        }
    }

    fprintf(stdout, "==========================\n");

    return ERR_NONE;
}

error_code client_cancel_look_for_game(client_state_t* state) {
    CancelLookForGameRequestMessage req;
    req.type = MSG_CANCEL_LOOK_FOR_GAME;
    strncpy(req.api_key, state->api_key, API_KEY_LEN);

    error_code err = send_message(state->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to send cancel look for game request\n" RESET, error_to_string(err));
        return err;
    }

    CancelLookForGameResponseMessage res;
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to read cancel look for game response\n" RESET, error_to_string(err));
        return err;
    }

    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: Cancel look for game request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_UNAUTHORIZED) {
            return ERR_UNATHORIZED;
        }

        return ERR_UNKNOWN;
    }

    return ERR_NONE;
}

error_code client_look_for_game(client_state_t* state) {
    LookForGameRequestMessage req;
    req.type = MSG_LOOK_FOR_GAME;
    strncpy(req.api_key, state->api_key, API_KEY_LEN);

    error_code err = send_message(state->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to send look for game request\n" RESET, error_to_string(err));
        return err;
    }

    LookForGameResponseMessage res;
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to read look for game response\n" RESET, error_to_string(err));
        return err;
    }

    if (res.error.status_code != STATUS_OK) {
        fprintf(stderr, RED "ERROR: Look for game request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_UNAUTHORIZED) {
            return ERR_UNATHORIZED;
        }

        return ERR_UNKNOWN;
    }

    fprintf(stdout, "Looking for game (press q to cancel)\n");

    struct pollfd poll_fds[2] = {
        { .fd = STDIN_FILENO, .events = POLLIN }, 
        { .fd = state->sock_fd, .events = POLLIN }, 
    };

    int timeout = 300;
    int dots_count = 0; 
    char cmd = 0;

    struct termios new, old;
    tcgetattr(STDIN_FILENO, &old); 
    new = old;

    // Set canonical mode so we get stdin event 
    // as soon as the user types something
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
     
    while (1) {
        int ret = poll(poll_fds, 2, timeout);
        if (ret == -1) {
            fprintf(stderr, RED "ERROR: poll failed %s\n" RESET, strerror(errno));
            break;
        } 

        // timeout 
        if (ret == 0) {
            // print 10 dots and then clear them
            if (dots_count == 10) {
                printf("\r          \r"); 
                fflush(stdout);
                dots_count = 0;
                continue;
            }
            putchar('.');
            fflush(stdout);
            dots_count++;

            continue;
        }
    

        // check if we have something in stdin
        if (poll_fds[0].revents & POLLIN) {
            cmd = getchar();

            if (err == ERR_IIN) {
                error_print(err);
                continue;
            }

            if (err == ERR_UNKNOWN) {
                error_print(err);
                break;
            }

            if (cmd == 'q') {
                break;
            } 

            fprintf(stderr, RED "ERROR: %c is invalid command\n" RESET, cmd);
            dots_count = 0;
        }

        if (poll_fds[1].revents & POLLIN) {
            fprintf(stdout, "GOT MESSAGE FROM SERVER\n");
            break;
        }
    }

    // return to normal mode
    tcsetattr(STDIN_FILENO, TCSANOW, &old);

    // Did user press q to cancel, if he did then send the cancel look for game request
    if (cmd == 'q') {
        // Print new line so that menu appears other dots
        fprintf(stdout, "\n");
        return client_cancel_look_for_game(state);
    }

    return ERR_NONE;
}


error_code client_challenge_player(client_state_t* state) {
	fprintf(stdout, "Enter player's username you whish to challenge (max %d): ", USERNAME_MAX_LEN);

    char target[USERNAME_MAX_LEN];
	error_code err = read_line(target, USERNAME_MAX_LEN);
	switch (err) {
	case ERR_NONE:
		break;
	case ERR_IIN:
	case ERR_UNKNOWN:
        error_print(err);
		return err;
	default:
		UNREACHABLE;
	}

    ChallengePlayerRequestMessage req;
    req.type = MSG_CHALLENGE_PLAYER;
    strncpy(req.api_key, state->api_key, API_KEY_LEN);
    strncpy(req.target_username, target, USERNAME_MAX_LEN);

    err = send_message(state->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to send challenge player request\n" RESET, error_to_string(err));
        return err;
    }

    ChallengePlayerResponseMessage res;
    err = read_message(state->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s Failed to read challenge player request\n" RESET, error_to_string(err));
        return err;
    }

    if (res.error.status_code != STATUS_OK) {
        if (res.error.status_code == STATUS_NOT_FOUND 
            || res.error.status_code == STATUS_PLAYER_IS_NOT_CONNECTED
            || res.error.status_code == STATUS_PLAYER_IS_NOT_LOOKING_FOR_GAME) {
            fprintf(stderr, RED "ERROR: %s\n" RESET, res.error.message);
            return err;
        }

        fprintf(stderr, RED "ERROR: Look for game request failed, %d - %s\n" RESET, res.error.status_code, res.error.message);

        if (res.error.status_code == STATUS_UNAUTHORIZED) {
            return ERR_UNATHORIZED;
        }

        return ERR_UNKNOWN;
    }

    fprintf(stdout, GREEN "Challenge accepted, entering lobby %d\n" RESET, res.success.lobby_id);

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
		err = menu_page_init(&page, 5);
		if (err != ERR_NONE) {
			return err;
		}

		{
			menu_item_t item = { .index = 1, .prompt = "List other players" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 2, .prompt = "Look for game" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 3, .prompt = "Challenge other player" };
			menu_page_add_item(&page, item);
		}
		{
			menu_item_t item = { .index = 4, .prompt = "Profile settings" };
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
