#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "arpa/inet.h"
#include <sys/time.h>
#include <tuple>

#include "../../headers/server/server.hpp"

#define PORT 8080


void initializeAllClients(int *clientSocket, int maxClients)
{
    for (int i = 0; i < maxClients; i++)
    {
        clientSocket[i] = 0;
    }
}


std::tuple<int, int> addChildSocketsToSet(
        int socketDescriptor,
        int maxMasterSocketDescriptor,
        int nMaxClients,
        const int *clientSocketArray,
        fd_set readFileDescriptor)
{
    for (int i = 0; i < nMaxClients; i++)
    {
        //socket descriptor
        socketDescriptor = clientSocketArray[i];

        //if valid socket descriptor then add to read list
        if (socketDescriptor > 0) FD_SET(socketDescriptor, &readFileDescriptor);

        //highest file descriptor number, need it for the select function
        if (socketDescriptor > maxMasterSocketDescriptor) maxMasterSocketDescriptor = socketDescriptor;
    }

    return std::make_tuple(socketDescriptor, maxMasterSocketDescriptor);
}


void startServer()
{
    struct sockaddr_in address{};

    int opt = true;
    int masterSocket, clientSocket[30], maxClients = 4, activity, valRead, sd, maxSd;

    int newSocket;
    int addrLen = sizeof(address);
    char buffer[1024] = {0};
    char *message = "ECHO Daemon v1.0 \r\n";

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //set of socket descriptors
    fd_set readFds;

    initializeAllClients(clientSocket, maxClients);

    // Creating socket file descriptor
    if ((masterSocket = socket(AF_INET,
                               SOCK_STREAM,
                               0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(masterSocket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   (char *) &opt,
                   sizeof(opt)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (bind(masterSocket,
             (struct sockaddr *) &address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Listener on port: %d \n", PORT);

    if (listen(masterSocket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    puts("Waiting for connections ...");

    while (true)
    {
        //clear the socket set
        FD_ZERO(&readFds);

        //add master socket to set
        FD_SET(masterSocket, &readFds);
        maxSd = masterSocket;

        addChildSocketsToSet(
                maxClients,
                sd,
                maxSd,
                clientSocket,
                readFds
                );

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(maxSd + 1, &readFds, nullptr, nullptr, nullptr);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then it's an incoming connection
        if (FD_ISSET(masterSocket, &readFds))
        {
            if ((newSocket = accept(masterSocket,
                                    (struct sockaddr *) &address,(socklen_t *) &addrLen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
                   newSocket,
                   inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port));

            //send new connection greeting message
            if (send(newSocket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (int i = 0; i < maxClients; i++)
            {
                //if position is empty
                if (clientSocket[i] == 0)
                {
                    clientSocket[i] = newSocket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        //else it's some IO operation on some other socket
        for (int i = 0; i < maxClients; i++)
        {
            sd = clientSocket[i];

            if (FD_ISSET(sd, &readFds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valRead = (int) read(sd, buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *) &address, \
                        (socklen_t *) &addrLen);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    clientSocket[i] = 0;
                }

                    //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valRead] = '\0';
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }

    }
}