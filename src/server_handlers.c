
#include "include/errors.h"
#include "include/globals.h"
#include "include/messages.h"
#include "include/state.h"
#include "include/users.h"
#include "include/vector/vector.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTE_MAX 256
void generate_random_hex_string(char* buffer, uint32_t len);

error_code handle_unknown_request(server_client_t* client) {
    ErrorResponseMessage res;
    res.status_code = STATUS_NOT_FOUND;
    sprintf(res.message, "Cannot process request, unknown message type");

    error_code  err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.status_code, res.message);
    }
    return err;
}

error_code handle_signup_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    SignupResponseMessage res = { 0 };

    if (client->flags & CLIENT_LOGGED_IN) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot signup when already logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    SignupRequestMessage req = *(SignupRequestMessage*)(buffer);
   
    pthread_mutex_lock(&client->state->users_lock);

    char found = 0;
    for (unsigned int i = 0; i < client->state->users.logical_length ; i++ ) {
         server_user_t* user = vector_at(&client->state->users, i);
         if (strncmp(req.username, user->username, USERNAME_MAX_LEN) == 0) {
             found = 1;
         }

         if (found) {
             break;
         }
    }

    pthread_mutex_unlock(&client->state->users_lock);

    if (found) {
        res.error.status_code = STATUS_CONFLICT;
        sprintf(res.error.message, "Username %s already exists", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User signup failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }
    
    strncpy(client->user.username, req.username, USERNAME_MAX_LEN);
    strncpy(client->user.password, req.password, PASSWORD_MAX_LEN);
    generate_random_hex_string(client->api_key, API_KEY_LEN);
    client->flags |= CLIENT_LOGGED_IN;

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" signed up\n" RESET, client->sock_fd,client->user.username);
   
    pthread_mutex_lock(&client->state->users_lock);
    vector_push(&(client->state->users), &client->user);
    pthread_mutex_unlock(&client->state->users_lock);
    
    res.success.status_code = STATUS_OK;
    strncpy(res.success.api_key, client->api_key, API_KEY_LEN);

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
    }
    return err;
}

error_code handle_login_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LoginResponseMessage res = { 0 };

    if (client->flags & CLIENT_LOGGED_IN) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot login when already logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    LoginRequestMessage req = *(LoginRequestMessage*)(buffer);
   
    pthread_mutex_lock(&client->state->users_lock);

    char found = 0;

    for (unsigned int i = 0; i < client->state->users.logical_length ; i++ ) {
         server_user_t* existing_user = vector_at(&client->state->users, i);
         if (strncmp(req.username, existing_user->username, USERNAME_MAX_LEN) == 0) {
             // Copy the user to the client
             client->user = *existing_user;
             found = 1;
         }

         if (found) {
             break;
         }
    }

    pthread_mutex_unlock(&client->state->users_lock);

    if (!found) {
        res.error.status_code = STATUS_NOT_FOUND;
        sprintf(res.error.message, "Failed to find user with username \"%s\"", req.username);
        fprintf(stderr, RED "ERROR: CLIENT %d: User login failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }
    
    if (strncmp(req.password, client->user.password, PASSWORD_MAX_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid password");
        fprintf(stderr, RED "ERROR: CLIENT %d: User login failed, %d - %s\n" RESET,
                client->sock_fd, res.error.status_code, res.error.message);

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    generate_random_hex_string(client->api_key, API_KEY_LEN);

    client->flags |= CLIENT_LOGGED_IN;

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" logged in\n" RESET, client->sock_fd,client->user.username);
    
    res.success.status_code = STATUS_OK;
    strncpy(res.success.api_key, client->api_key, API_KEY_LEN);

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
    }
    return err;
}

error_code handle_logout_request(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LogoutResponseMessage res = { 0 };

    if (!(client->flags & CLIENT_LOGGED_IN)) {
        res.error.status_code = STATUS_BAD_REQUEST;
        sprintf(res.error.message, "Cannot logout while not logged in");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    LogoutRequestMessage req = *(LogoutRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d - %s" RESET, error_to_string(err), res.error.status_code, res.error.message);
        }
        return err;
    }

    fprintf(stderr, GREEN "CLIENT %d: User \"%s\" logged out\n" RESET, client->sock_fd,client->user.username);

    client->flags &= ~CLIENT_LOGGED_IN;
    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
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

    if (!(client->flags & CLIENT_LOGGED_IN)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    ListUsersRequestMessage req = *(ListUsersRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    res.success.status_code = STATUS_OK;
    // We will send len - 1 users back because we are not sending this user
    // that sent the request
    res.success.count = 0;


    pthread_mutex_lock(&client->state->clients_lock);
   
    // Count clients that are logged in and skip this client
    for (uint32_t i = 0; i < client->state->clients.logical_length; i++) {
        server_client_t* other= vector_at(&client->state->clients, i);
        if (!(other->flags & CLIENT_LOGGED_IN)) {
            continue; 
        }

        if (client == other) {
            continue;
        }

        res.success.count++;
    } 

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count" RESET, client->sock_fd);

        pthread_mutex_unlock(&client->state->clients_lock);
        return err;
    }

    for (uint32_t i = 0; i < client->state->clients.logical_length; i++) {
        server_client_t* other= vector_at(&client->state->clients, i);

        // skip clients that are not logged in
        if (!(other->flags & CLIENT_LOGGED_IN)) {
            continue; 
        }

        // Skip user that sent the request
        if (client == other) {
            continue;
        }

        uint8_t looking_for_game = 0;
        if (other->flags & CLIENT_LOOKING_FOR_GAME) {
            looking_for_game = 1;
        }

        err = send_message(client->sock_fd, &looking_for_game, sizeof(looking_for_game));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send %s looking for game byte" RESET, client->sock_fd, client->user.username);

            pthread_mutex_unlock(&client->state->users_lock);
            return err;
        }

        err = send_message(client->sock_fd, other->user.username, USERNAME_MAX_LEN);
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send %s username" RESET, client->sock_fd, client->user.username);

            pthread_mutex_unlock(&client->state->clients_lock);
            return err;
        }
    }

    pthread_mutex_unlock(&client->state->clients_lock);
    return ERR_NONE;
}

error_code handle_look_for_game(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    LookForGameResponseMessage res = { 0 };

    if (!(client->flags & CLIENT_LOGGED_IN)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    LookForGameRequestMessage req = *(LookForGameRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    client->flags |= CLIENT_LOOKING_FOR_GAME;

    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count" RESET, client->sock_fd);
        return err;
    }

    return ERR_NONE;
}

error_code handle_cancel_look_for_game(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    CancelLookForGameResponseMessage res = { 0 };

    if (!(client->flags & CLIENT_LOGGED_IN)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    CancelLookForGameRequestMessage req = *(CancelLookForGameRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    client->flags &= ~CLIENT_LOOKING_FOR_GAME;

    res.success.status_code = STATUS_OK;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count" RESET, client->sock_fd);
        return err;
    }

    return ERR_NONE;
}

error_code handle_challenge(server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    ChallengeResponseMessage res = { 0 };

    if (!(client->flags & CLIENT_LOGGED_IN)) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    ChallengeRequestMessage req = *(ChallengeRequestMessage*)(buffer);
    if (strncmp(req.api_key, client->api_key, API_KEY_LEN) != 0) {
        res.error.status_code = STATUS_UNAUTHORIZED;
        sprintf(res.error.message, "Invalid api token");

        err = send_message(client->sock_fd, &res, sizeof(res));
        if (err != ERR_NONE) {
            fprintf(stderr, RED "%s failed to send message %d" RESET, error_to_string(err), res.success.status_code);
        }
        return err;
    }

    return ERR_NONE;
}

