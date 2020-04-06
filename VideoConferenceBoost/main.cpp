#include "Client.h"

#if defined(_DEBUG) || !defined(_WIN32)
int main()
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    try
    {
        Client client;
        client.start();
    }
    catch(std::exception exception)
    {
        if (strcmp(exception.what(), "Webcam Error") == 0)
        {
            return 0;
        }
    }
    return 0;
}