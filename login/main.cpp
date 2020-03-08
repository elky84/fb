#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include "socket.h"
#include "fb_login.h"
#include <Windows.h>
#include "leak.h"
#include "console.h"
#include "resource.h"

#pragma comment(lib, "ws2_32.lib")
#if defined DEBUG | defined _DEBUG
#pragma comment(lib, "json/json_vc71_libmtd.lib")
#pragma comment(lib, "zlib/zlibd.lib")
#else
#pragma comment(lib, "json/json_vc71_libmt.lib")
#pragma comment(lib, "zlib/zlib.lib")
#endif

using namespace fb::login;

acceptor*       acceptor_login;

BOOL WINAPI handle_console(DWORD signal)
{
    switch(signal)
    {
    case CTRL_C_EVENT:
        acceptor_login->exit();
        puts("Please wait to exit acceptor.");
        break;
    }

    return true;
}

int main(int argc, const char** argv)
{
	//_CrtSetBreakAlloc(165);

	::SetConsoleIcon(IDI_BARAM);
	::SetConsoleTitle(CONSOLE_TITLE);
    ::SetConsoleCtrlHandler(handle_console, true);

    // Initialization
    WSADATA                 wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 0;


    // Execute acceptor
    acceptor_login = new acceptor(2001);
    acceptor_login->execute(true);

    fgetchar();

    // Clean up
    delete acceptor_login;
    WSACleanup();

    return 0;
}