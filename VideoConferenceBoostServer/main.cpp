#include "Server.h"

constexpr unsigned short serverPort = 50005;

int main()
{
    Server server(serverPort);
    server.start();
    return 0;
}