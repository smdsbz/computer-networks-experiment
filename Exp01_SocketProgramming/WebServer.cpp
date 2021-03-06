#include "pch.h"
#include <iostream>
using std::cout; using std::cin; using std::endl;
#include <csignal>
using std::signal;
#include <stdexcept>

#include "Server.h"

Server *pserver = 0;
volatile bool running = false;


BOOL WINAPI HandlerRoutine(DWORD ctrlType) {
    switch (ctrlType) {
        case CTRL_C_EVENT: {
            cout << "<Ctrl-C> caught!" << endl;
            if (running == false) {
                cout << "Server not running!" << endl;
            }
            running = false;
            return TRUE;
        }
        default: { break; }
    }
    return FALSE;
}


int __main(void)
{
    if (!SetConsoleCtrlHandler(HandlerRoutine, TRUE)) {
        std::cerr << "Failed to install Control Key Hander!" << endl;
        return -1;
    }

    while (true) {
        cout << "Creating new server..." << endl;
        pserver = new Server(running);
        cout << "Entering service loop..." << endl;
        try {
            running = true;
            Server::StartServiceLoop((LPVOID)pserver);
            delete pserver;
        } 
        catch (std::runtime_error &e) {
            cout << e.what() << endl;
        }
        break;
    }

    cout << "\nPress any key to exit!" << endl;
    getchar();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
