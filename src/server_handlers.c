
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

    if (client->logged_in) {
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
    client->logged_in = 1;

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

    if (client->logged_in) {
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
    client->logged_in = 1;

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

    if (!client->logged_in) {
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

    client->logged_in = 0;
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
}

error_code handle_list_users (server_client_t* client, const char* buffer) {
    error_code err = ERR_NONE;
    ListUsersResponseMessage res = { 0 };

    if (!client->logged_in) {
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
    res.success.count = client->state->users.logical_length - 1;

    err = send_message(client->sock_fd, &res, sizeof(res));
    if (err != ERR_NONE) {
        fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send list users count" RESET, client->sock_fd);
        return err;
    }

    pthread_mutex_lock(&client->state->users_lock);
    
    for (uint32_t i = 0; i < client->state->users.logical_length; i++) {
        server_user_t* user = vector_at(&client->state->users, i);
        
        // Skip user that sent the request
        if (strncmp(user->username, client->user.username, USERNAME_MAX_LEN) == 0) {
            continue;
        }

        err = send_message(client->sock_fd, user->username, USERNAME_MAX_LEN);
        if (err != ERR_NONE) {
            fprintf(stderr, RED "ERROR: CLIENT %d: Failed to send %s username" RESET, client->sock_fd, user->username);

            pthread_mutex_unlock(&client->state->users_lock);
            return err;
        }
    }

    pthread_mutex_unlock(&client->state->users_lock);
    return ERR_NONE;
}
