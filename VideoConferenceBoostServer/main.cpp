#include "Server.h"

Server server;

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch (CEvent)
    {
    case CTRL_C_EVENT:
    {
        server.stop();
        break;
    }
    case CTRL_BREAK_EVENT:
    {
        server.stop();
        break;
    }
    case CTRL_CLOSE_EVENT:
    {
        server.stop();
        break;
    }
    case CTRL_LOGOFF_EVENT:
    {
        server.stop();
        break;
    }
    case CTRL_SHUTDOWN_EVENT:
    {
        server.stop();
        break;
    }
    }
    return TRUE;
}

int main()
{
    system("chcp 1251 > nul");
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
    {
        printf("Unable to install handler!\n");
        return -1;
    }
    server.start();
    return 0;
}