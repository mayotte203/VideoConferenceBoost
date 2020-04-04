#include "Client.h"
#ifdef _DEBUG
int main()
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    system("chcp 1251 > nul");
    
    Client client;
    client.start();
}