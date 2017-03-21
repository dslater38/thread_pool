
#ifdef WIN32
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
 #include <netdb.h>

#endif


namespace native
{
    using Socket=decltype(::socket(AF_INET, SOCK_STREAM, 0));
    void init();
    void shutdown();
    int closesocket(Socket);
}

