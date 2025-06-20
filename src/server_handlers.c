#include "include/errors.h"
#include "include/game.h"
#include "include/globals.h"
#include "include/server_utils.h"
#include "include/messages.h"
#include "include/state.h"
#include "include/users.h"
#include "include/vector/vector.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static error_code handle_ask_other_player(server_client_t* client, server_client_t* other);

#define BYTE_MAX 256
void generate_random_hex_string(char* buffer, uint32_t len);

error_code handle_unknown_request(server_client_t* client) {
    ErrorResponseMessage res;
    res.status_code = STATUS_NOT_FOUND;
    sprintf(res.message, "Cannot process request, unknown message type");

    error_code  err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.status_code, res.message);
    }
    return err;
}

error_code handle_signup_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    SignupResponseMessage res = { 0 };

    if (client_logged_in(client)) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot signup when already logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    SignupRequestMessage req = *(SignupRequestMessage*)(buffer);
   
    server_user_t* user = server_find_user_by_username(client->server_state, req.username);
    if (user != NULL) {
        res.error.status_code = STATUS_CONFLICT;
        sprintf(res.error.message, "Username %s already exists", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User signup failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }
    
    server_user_t new_user;
    strncpy(new_user.username, req.username, USERNAME_MAX_LEN);
    strncpy(new_user.password, req.password, PASSWORD_MAX_LEN);

    client->user = server_add_user(client->server_state, new_user);

    generate_random_hex_string(client->api_key, API_KEY_LEN);

    client_set_logged_in(client);

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" signed up\n" RESET, client->sock_fd,client->user->username);
   
    res.success.status_code = STATUS_OK;
    strncpy(res.success.api_key, client->api_key, API_KEY_LEN);

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d\n" RESET, error_to_string(err), res.success.status_code);
    }
    return err;
}

error_code handle_login_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LoginResponseMessage res = { 0 };

    if (client_logged_in(client)) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot login when already logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    LoginRequestMessage req = *(LoginRequestMessage*)(buffer);
  
    // Check if there is a client that is logged in with current user
    server_client_t* other = server_find_client_by_username(client->server_state, req.username);
    if (other == client) {
        // We already checked if we are logged in and because we are not logged in
        // our user is currently NULL so this is unreachable
        UNREACHABLE;
    }

    if (other != NULL) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot login because somebody else is already logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    
    server_user_t* user = server_find_user_by_username(client->server_state, req.username);
    if (user == NULL) {
        res.error.status_code = STATUS_NOT_FOUND;
        sprintf(res.error.message, "Failed to find user with username \"%s\"", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User login failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }
    
    if (strncmp(req.password, user->password, PASSWORD_MAX_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid password");
        fprintf(stderr, RED "ERROR: CLIENT %d: User login failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    client->user = user;
    generate_random_hex_string(client->api_key, API_KEY_LEN);
    client_set_logged_in(client);
     
    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" logged in\n" RESET, client->sock_fd, user->username);
    
    res.success.status_code = STATUS_OK;
    strncpy(res.success.api_key, client->api_key, API_KEY_LEN);

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d\n" RESET, error_to_string(err), res.success.status_code);
    }
    return err;
}

error_code handle_logout_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LogoutResponseMessage res = { 0 };

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot logout while not logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    LogoutRequestMessage req = *(LogoutRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s\n" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" logged out\n" RESET, client->sock_fd,client->user->username);

    client->user = NULL;
    client_clear_logged_in(client);
    memset(client->api_key, 0, API_KEY_LEN);

    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d\n" RESET, error_to_string(err), res.success.status_code);
    }

    return err;
}

void generate_random_hex_string(char* buffer, uint32_t len) {
    uint32_t half = len / 2;

    for (size_t i = 0; i < half; i++) {
        uint8_t byte = (uint8_t)(rand() % BYTE_MAX);
        sprintf(buffer + (i * 2), "%02x", byte);
    }

    buffer[len] = '\0';
}

error_code handle_list_users(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    ListUsersResponseMessage res = { 0 };

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    ListUsersRequestMessage req = *(ListUsersRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    res.success.status_code = STATUS_OK;
    // We will send len - 1 users back because we are not sending this user
    // that sent the request
    res.success.count = 0;

    pthread_rwlock_rdlock(&client->server_state->clients_rwlock);
   
    // Count clients that are logged in and skip this client
    for (uint32_t i = 0; i < client->server_state->clients.logical_length; i++) {
        server_client_t* other= vector_at(&client->server_state->clients, i);
        if (!client_logged_in(other)) {
            continue; 
        }

        if (client == other) {
            continue;
        }

        res.success.count++;
    } 

    pthread_rwlock_unlock(&client->server_state->clients_rwlock);

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s CLIENT %d: Failed to send list users count\n" RESET, error_to_string(err), client->sock_fd);
        return err;
    }

    pthread_rwlock_rdlock(&client->server_state->clients_rwlock);

    for (uint32_t i = 0; i < client->server_state->clients.logical_length; i++) {
        server_client_t* other = vector_at(&client->server_state->clients, i);

        // Skip clients that are not logged in
        if (!client_logged_in(other)) {
            continue; 
        }

        // Skip user that sent the request
        if (client == other) {
            continue;
        }

        uint8_t looking_for_game = 0;
        if (client_looking_for_game(other)) {
            looking_for_game = 1;
        }

        err = send_message(client->sock_fd, &looking_for_game, sizeof(looking_for_game));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send looking for game byte\n" RESET, error_to_string(err), client->sock_fd);
            break;
        }

        err = send_message(client->sock_fd, other->user->username, USERNAME_MAX_LEN);
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send username\n" RESET, error_to_string(err), client->sock_fd);
            break;
        }
    }

    pthread_rwlock_unlock(&client->server_state->clients_rwlock);

    return ERR_NONE;
}

error_code handle_look_for_game(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LookForGameResponseMessage res = { 0 };

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    LookForGameRequestMessage req = *(LookForGameRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    client_set_looking_for_game(client);

    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        return err;
    }

    return ERR_NONE;
}

error_code handle_cancel_look_for_game(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    CancelLookForGameResponseMessage res = { 0 };

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Client is not logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    CancelLookForGameRequestMessage req = *(CancelLookForGameRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        }
        return err;
    }

    client_clear_looking_for_game(client);

    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s CLIENT %d: Failed to send message\n" RESET, error_to_string(err), client->sock_fd);
        return err;
    }

    return ERR_NONE;
}

error_code handle_challenge_player(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    ChallengePlayerResponseMessage res = { 0 };

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count\n" RESET, client->sock_fd);
        }
        return err;
    }

    ChallengePlayerRequestMessage req = *(ChallengePlayerRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count\n" RESET, client->sock_fd);
        }

        return err;
    }

    
    // Go through all connected clients and check if one of them matches the requested one and its looking for game
    server_client_t* other = server_find_client_by_username(client->server_state, req.target_username);
    if (client == other) {
        // We cannot at the same time look for game and challenge others so
        // this is unreachable;
        UNREACHABLE;
    }
    
    if (other == NULL) {
        res.error.status_code = STATUS_NOT_FOUND;
        sprintf(res.error.message, "Player \"%s\" doesn't exist", req.target_username);
        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message %d\n" RESET, error_to_string(err), client->sock_fd, res.success.status_code);
        }
        return err;
    }

    if (!client_logged_in(other)) {
        res.error.status_code = STATUS_PLAYER_IS_NOT_CONNECTED;
        sprintf(res.error.message, "Player \"%s\" is not connected", other->user->username);
        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message %d\n" RESET, error_to_string(err), client->sock_fd, res.success.status_code);
        }
        return err;
    }

    if (!client_looking_for_game(other)){
        res.error.status_code = STATUS_PLAYER_IS_NOT_LOOKING_FOR_GAME;
        sprintf(res.error.message, "Player \"%s\" is not looking a for game", other->user->username);
        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message %d\n" RESET, error_to_string(err), client->sock_fd, res.success.status_code);
        }
        return err;
    }


    fprintf(stdout, "CLIENT %d: Asking other player does he want to play\n", client->sock_fd);

    // Ask other player does he want to play
    err = handle_ask_other_player(client, other);
    if (err != ERR_NONE) {
        res.error.status_code = STATUS_PLAYER_ERROR;
        sprintf(res.error.message, "Player \"%s\" failed to respond successfully", other->user->username);
        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: Failed to send message %d\n" RESET, error_to_string(err), client->sock_fd, res.success.status_code);
        }
        return err;
    }

    // Send the message to the other player when he responds we will get that requets in 
    // handle_client function and should process it there because its only valid if we 
    // read messages from one place to avoid race conditions.
    //
    // To achieve this, create new game lobby with id and add both clients there

    ServerGame* game = server_add_game(client->server_state, game_new(client, other));
    
    client_join_game(client, game);
    client_join_game(other, game);

    // Client that started the challenge automatically acceptes the game
    game_accept(game, client);

    return ERR_NONE;
}

static error_code handle_ask_other_player(server_client_t* client, server_client_t* other) {
    ChallengeQuestionRequestMessage req;
    req.type = MSG_CHALLENGE_QUESTION;
    strncpy(req.challenger_username, client->user->username, USERNAME_MAX_LEN);

    error_code err = send_message(other->sock_fd, &req, sizeof(req));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send accept challenge request\n" RESET, error_to_string(err));
        return err;
    }

    return ERR_NONE; 
}

error_code handle_challenge_answer(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    ChallengePlayerResponseMessage res = {0};

    if (!client_logged_in(client)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count\n" RESET, client->sock_fd);
        }
        return err;
    }

    ChallengeAnswerRequestMessage req = *(ChallengeAnswerRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count\n" RESET, client->sock_fd);
        }

        return err;
    }

    ServerGame* game = client->game;
    server_client_t* other = game_other_player(game, client);

    if (req.accept) {
        res.success.status_code = STATUS_OK;
        res.success.game_id = client->game->id;
    
        game_accept(game, client);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: failed to send message\n" RESET, error_to_string(err), client->sock_fd);
            return err;
        }

        err = send_message(other->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s CLIENT %d: failed to send message\n" RESET, error_to_string(err), other->sock_fd);
            return err;
        }

        return ERR_NONE;
    }

    res.error.status_code = STATUS_PLAYER_DECLINED;
    sprintf(res.error.message, "Player \"%s\" declined the challenge", client->user->username);

    err = send_message(other->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s CLIENT %d: failed to send message\n" RESET, error_to_string(err), other->sock_fd);
        return err;
    }

    client->game = NULL;
    other->game = NULL;

    server_remove_game(client->server_state, game);

    return ERR_NONE;
}
