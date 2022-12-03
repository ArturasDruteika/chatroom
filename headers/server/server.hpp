#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <tuple>


void initializeAllClients(int *clientSocket, int maxClients);
std::tuple<int, int> addChildSocketsToSet(
        int socketDescriptor,
        int maxMasterSocketDescriptor,
        int nMaxClients,
        const int *clientSocketArray,
        fd_set readFileDescriptor);
void startServer();