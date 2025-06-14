#include "include/messages.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <include/errors.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>

error_code send_message(int sock_fd, const void* message, uint32_t len) {
    ssize_t sent = send(sock_fd, message, len, 0);
    if ((uint32_t)sent == len) {
        fprintf(stderr, ">>>>> %d: Sent a message, len %ld\n", sock_fd, sent);
        return ERR_NONE;
    }

    switch (errno) {
    case ENOTSOCK:
        // File descriptor passed is not socket
    case ENOTCONN:
        // Socket is not in connected
    case EDESTADDRREQ:
        // Socket destination is not set
    case EBADF:
        // Socket is not a valid open fd
        return ERR_IFD;
    case ENOMEM:
        // No memory to send a message
        return ERR_SEND_NO_MEM;
    default:
        return ERR_UNKNOWN;
    }
}


error_code read_message(int sock_fd, void* buffer, uint32_t len) {
    ssize_t read = recv(sock_fd, buffer, len, 0);
    if (read == 0) {
        return ERR_PEER_CLOSED;
    }
    
    if (read != -1 && (uint32_t)read <= len) {
        fprintf(stderr, "<<<<< %d: Read a message, len %ld\n", sock_fd, read);
        return ERR_NONE;
    }

    switch (errno) {
    case EFAULT:
        // Buffer points out of process's memory address space
    case EINVAL:
        // Invalid argument passed
        return ERR_IARG;
    case ECONNREFUSED:
        // Remote peer refused to connect
        // Probably unreachable
    case EBADF:
        // Socket is not a valid open fd
    case ENOTSOCK:
        // File descriptor passed is not socket
        return ERR_IFD;
    default:
        return ERR_UNKNOWN;
    }
}

