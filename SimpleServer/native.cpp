#ifdef WIN32

#include <Winsock2.h>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace native {
    
    
    void init()
    {
        WSADATA wsaData;
        // Initialize Winsock
        auto iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
            std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
            _exit(1);
        }
        
    }
    void shutdown()
    {
        WSACleanup();
    }
    
}
#else
namespace native {
    
    
    void init()
    {
    }
    void shutdown()
    {
    }
}

#endif

