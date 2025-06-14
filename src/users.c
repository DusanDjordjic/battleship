#include <include/users.h>
#include "include/errors.h"
#include "include/globals.h"
#include "include/state.h"
#include "include/vector/vector.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


error_code users_load(Vector* users, const char* filepath) {
    FILE* file = fopen(filepath, "ab+");;
    if (file == NULL) {
        return ERR_UNKNOWN;
    }

    fseek(file, 0, SEEK_END);
    int64_t isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }
    
    uint64_t size = (uint64_t)isize; 

    fseek(file, 0, SEEK_SET);

    // Check if fseek failed
    isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    if (isize != 0) {
        fprintf(stderr, RED "ERROR: After calling fseek to set cursor at the beginning ftell returned %ld\n" RESET, isize);
        fclose(file);
        return ERR_UNKNOWN;
    }

    uint64_t user_count = size / sizeof(server_user_t);
    if (user_count == 0) {
        fprintf(stdout, "No users to load, skipping...\n");
        vector_create(users, sizeof(server_user_t));
        fclose(file);
        return ERR_NONE;
    }

    // allocate memory for all users
    users->elements = malloc(sizeof(server_user_t) * user_count);
    if (users->elements == NULL) {
        fclose(file);
        return ERR_ALLOC;
    }

    users->element_size = sizeof(server_user_t); 
    users->logical_length = user_count;
    users->actual_length = user_count;

    uint64_t count = fread(users->elements, sizeof(server_user_t), user_count, file);
    if (ferror(file)) {
        fprintf(stderr, RED "ERROR: Failed to read all users: expected %lu, read %lu\n" RESET, user_count, count);
        fclose(file);
        return ERR_UNKNOWN;
    }

    if (count == user_count) {
        fprintf(stdout, "Loaded all %lu users, total file size %lu\n" RESET, user_count, size);
        fclose(file);
        return ERR_NONE;
    }

    // We didn't reach the end of file 
    fprintf(stderr, RED "ERROR: Didn't reach the end of file, users to read: expected %lu, read %lu\n" RESET, user_count, count);
    fclose(file);
    return ERR_UNKNOWN;
}

error_code users_save(Vector* users, const char* filepath) {
    FILE* file = fopen(filepath, "w");;
    if (file == NULL) {
        return ERR_UNKNOWN;
    }

    fseek(file, 0, SEEK_SET);
    int64_t isize = ftell(file);
    if (isize == -1) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    fwrite(users->elements, sizeof(server_user_t), users->logical_length, file);
    if (ferror(file)) {
        fclose(file);
        return ERR_UNKNOWN;
    }

    fprintf(stdout, "Saved %u users\n" RESET, users->logical_length);

    fclose(file);
    return ERR_NONE;
}
