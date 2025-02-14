#ifndef STATE_H
#define STATE_H

#include "vector/vector.h"
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <stdint.h>

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

typedef struct {
    int sock_fd; 
	uint16_t port;
    Vector clients;
    pthread_mutex_t clients_lock;
} server_state_t;

#endif
