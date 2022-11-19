#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "arpa/inet.h"

#include "../../headers/server/server.hpp"


#define PORT 8080


void startServer()
{
    struct sockaddr_in address{};

    int server_fd, new_socket;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    std::string welcomeToServerMsg = "Hello from server";
    char *hello = const_cast<char *>(welcomeToServerMsg.c_str());

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET,
                            SOCK_STREAM,
                            0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd,
             (struct sockaddr *) &address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd,
                             (struct sockaddr *) &address,
                             (socklen_t *) &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    while(true)
    {
        bzero(buffer, 256);
        read(new_socket, buffer, 1024);

        printf("IP address is (client name): %s, PORT: %u\n", inet_ntoa(address.sin_addr), (int)address.sin_port);

        printf("%s\n", buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");

        // closing the connected socket
        if (i == 10) break;
        i += 1;
    }

    close(new_socket);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
}