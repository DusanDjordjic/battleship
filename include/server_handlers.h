#ifndef SERVER_HANDLERS_H
#define SERVER_HANDLERS_H

#include <include/messages.h>
#include <include/state.h>

error_code handle_unknown_request(server_client_t* client);
error_code handle_signup_request(server_client_t* client, const char* buffer);
error_code handle_login_request(server_client_t* client, const char* buffer);
error_code handle_logout_request(server_client_t* client, const char* buffer);
error_code handle_list_users(server_client_t* client, const char* buffer);
error_code handle_look_for_game(server_client_t* client, const char* buffer);
error_code handle_cancel_look_for_game(server_client_t* client, const char* buffer);
error_code handle_challenge_player(server_client_t* client, const char* buffer);

#endif
