#include "Client.h"
#ifdef _DEBUG
int main()
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    system("chcp 1251 > nul");
    try
    {
        Client client;
        client.start();
    }
    catch(std::exception exception)
    {
        if (strcmp(exception.what(), "Webcam Error") == 0)
        {
            MessageBox(
                NULL,
                (LPCWSTR)L"Webcam Error",
                (LPCWSTR)L"Can't open webcam",
                MB_ICONERROR | IDOK
            );
            return 0;
        }
    }
    return 0;
}