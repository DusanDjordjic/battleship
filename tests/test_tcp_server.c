#include <include/criterion/criterion.h>
#include <include/criterion/logging.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <include/server.h>
#include <include/errors.h>

#define TEST_PORT 8080

Test(tcp_server, starts_successfully) {
    int server_fd = server_start(TEST_PORT);
    cr_assert(server_fd > 0, "Failed to start server!");

    // Test if the server is listening
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    cr_assert(client_fd > 0, "Failed to create client socket!");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TEST_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    int connection_status = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    cr_assert_eq(connection_status, 0, "Failed to connect to server!");

    close(client_fd);
    close(server_fd);
}
