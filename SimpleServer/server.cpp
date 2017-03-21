/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
   
#include "tcp_streambuf.h"
#include "thread_pool.h" 
#include "native.h"



#include <thread>
#include <string>
#include <cstring>
#include <set>
#include <vector>
#include <sstream>


#define PSEND 0
#define PRECV 1

using Socket = native::Socket;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void
ll_Log(std::iostream &ss)
{
	ss << std::endl;
}

template<typename T, typename ...Targs >
void
ll_Log(std::iostream &ss, T t, Targs...args)
{
	ss << t;
	ll_Log(ss, std::forward<Targs>(args)...);
}

#ifdef _DEBUG

template<typename ...Targs >
void
Log(Targs...args)
{
	std::stringstream ss{};
	ss << std::hex << std::this_thread::get_id() << ": ";
	ll_Log(ss, std::forward<Targs>(args)...);
	std::cerr << ss.str();
}

#else
template<typename ...Targs >
void
Log(Targs...) {}
#endif

static
std::array<native::Socket, 2>
tpipe();

std::set<Socket>
getListeners(const char *port);

// this runs in the main thread and performs all maintainence
// on master_set, readFds, & listenerFds
int
handleFdSetSignal(Socket signalFd, const std::set<Socket> &listenerFds, std::set<Socket> &readFds, fd_set *readset, fd_set *master_set);

// Handle incoming connection requests. This dispatches the connection request to the thread pool.
void
handleIncomingConnections(thread_pool &pool_, Socket sendFd, const std::set<Socket> &listenFds, fd_set *readset, fd_set *master_set);

// handle incoming data requests. This dispatches to the thread pool.
void
handleIncomingData(thread_pool &pool_, Socket sendFd, std::set<Socket> &readFds, fd_set *readset, fd_set *master_set);

int main(int argc, char *argv[])
{
	native::init();


	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	auto maxFd = 0;
	auto listenFds = getListeners(argv[1]);
	auto readFds = std::set<native::Socket>{};
	if (!listenFds.empty()) {
		auto pipe = tpipe();
		Log("pipe[PRECV]: ", pipe[PRECV]);
		Log("pipe[PSEND]: ", pipe[PSEND]);
		fd_set master_set;
		fd_set readset;
		FD_ZERO(&master_set);

		maxFd = (int)(pipe[PRECV]);

		FD_SET(pipe[PRECV], &master_set);

		for (auto fd : listenFds) {
			maxFd = std::max(maxFd, (int)fd);
			FD_SET(fd, &master_set);
		}

		thread_pool	pool_{};

		for (;;) {
			FD_ZERO(&readset);
			readset = master_set;
			auto result = select(maxFd + 1, &readset, nullptr, nullptr, 0);
			if (result > 0) {
				maxFd = std::max(maxFd, handleFdSetSignal(pipe[PRECV], listenFds, readFds, &readset, &master_set));
				handleIncomingConnections(pool_, pipe[PSEND], listenFds, &readset, &master_set);
				handleIncomingData(pool_, pipe[PSEND], readFds, &readset, &master_set);
			}
			else {
				Log("select() failed.");
				break;
			}
		}
	}
	native::shutdown();
	return 0;

}

#if 0
int main(int argc, char *argv[])
{
	native::init();


	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}


    struct addrinfo hints;
    struct addrinfo *result = nullptr;
    std::memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    auto retVal = getaddrinfo( nullptr, argv[1], &hints, &result );
    
    if( !retVal )
    {
        
		thread_pool	pool_{};

      /* getaddrinfo() returns a list of address structures.
          Try each address until we successfully connect(2).
          If socket(2) (or connect(2)) fails, we (close the socket
          and) try the next address. */
       
		auto listenFds = std::set<native::Socket>{};
		auto readFds = std::set<native::Socket>{};
		auto pipe = tpipe();
		auto signalFd = pipe[PRECV];
		auto sendFd = pipe[PSEND];
		Log("pipe[PRECV]: ", signalFd);
		Log("pipe[PSEND]: ",sendFd);
		auto maxFd = (native::Socket)0;
		// listenFds.emplace(signalFd);
       for (auto *rp = result; rp != nullptr; rp = rp->ai_next) 
         {
           auto listenFd = socket(rp->ai_family, rp->ai_socktype,
                   rp->ai_protocol);
            
           if (listenFd == INVALID_SOCKET)
               continue;

           if (::bind(listenFd, rp->ai_addr, static_cast<int>(rp->ai_addrlen)) == 0)
           {
			   maxFd = std::max(maxFd, listenFd);
			   Log("listenFd: ", listenFd);
			   listenFds.emplace(listenFd);
           }
		   else
		   {
			   close(listenFd);
		   }
       }

	   freeaddrinfo(result);

	   if (!listenFds.empty())
	   {
		   fd_set master_set;
		   fd_set readset;
		   FD_ZERO(&master_set);
		   FD_SET(signalFd, &master_set);
			for (auto fd : listenFds)
			{
				listen(fd, 5);
				FD_SET(fd, &master_set);
			}

		   for(;;){
			   FD_ZERO(&readset);
			   readset = master_set;
			   auto result = select(static_cast<int>(maxFd) + 1, &readset, nullptr, nullptr, 0);
			   if (result > 0)
			   {
				   if (FD_ISSET(signalFd, &readset))
				   {
					   handleFdSetSignal(signalFd, listenFds, readFds, &master_set);
				   }
				   for (auto fd : listenFds)
				   {
					   if (FD_ISSET(fd, &readset))
					   {
						   acceptConnection()
						   FD_CLR(fd, &master_set);
						   Log("main: clr: ", fd);
						   pool_.enqueueWork([sendFd,fd](thread_pool &) {

							   native::Socket fds [2];
							   fds[0] = fd;
							   fds[1] = accept(fd, nullptr, nullptr);
							   auto sendSize = (fds[1] != INVALID_SOCKET) ? 2 : 1;
							   if (sendSize == 2)
								   Log("accept: Sending: ", fds[0], ", ", fds[1]);
							   else
								   Log("accept: Sending: ", fds[0]);;

							   auto result = ::send(sendFd, (char *)fds, (int)sizeof(native::Socket)*sendSize, 0);
							   Log("accept: Sent: ", result, " bytes ");
						   });
					   }
				   }
				   for (auto fd : readFds)
				   {
					   if (FD_ISSET(fd, &readset))
					   {
						   FD_CLR(fd, &master_set);
						   Log("readReady: clr: ", fd);
						   pool_.enqueueWork([=](thread_pool &p) {
							   std::shared_ptr<char> buf{ new char[256] };;
							   auto result = ::recv(fd, buf.get(), 256, 0);
							   Log("readReady: recv: ", fd);
							   if (result > 0)
							   {
								   p.enqueueWork([=](thread_pool &p) {
									   Log("readReady: send: ", fd);
									   ::send(fd, buf.get(), result, 0);
								   });
								   Log("readReady: signal: ", fd);
								   auto result = ::send(sendFd, (char *)&fd, (int)sizeof(fd), 0);
								   Log("readReady: Sent: ", result , " bytes ");
							   }
							   else
							   {
								   if (result == 0)
								   {
									   // client close
									   Log("connection closed by client");
								   }
								   else if (result < 0)
								   {
									   Log("Error reading from client");
								   }
							   }
						   });
					   }
				   }
			   }
			   else
			   {
				   Log("select() failed." );
				   break;
			   }
		   }
	   }
	}
    else
    {
        Log("getaddrinfo() failed.");
    }
   native::shutdown();
    return 0;
}
#endif // 0

std::set<Socket>
getListeners(const char *port)
{
	std::set<Socket> listenFds{};
	struct addrinfo hints;
	struct addrinfo *result = nullptr;
	std::memset(&hints, '\0', sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	auto retVal = getaddrinfo(nullptr, port, &hints, &result);

	if (!retVal && result)
	{
		/* getaddrinfo() returns a list of address structures.
		Try each address until we successfully connect(2).
		If socket(2) (or connect(2)) fails, we (close the socket
		and) try the next address. */

		for (auto *rp = result; rp != nullptr; rp = rp->ai_next)
		{
			auto listenFd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);

			if (listenFd == INVALID_SOCKET)
				continue;

			if (::bind(listenFd, rp->ai_addr, static_cast<int>(rp->ai_addrlen)) == 0)
			{
				listen(listenFd, 5);
				listenFds.emplace(listenFd);
			}
			else
			{
				close(listenFd);
			}
		}
		freeaddrinfo(result);
	}
	return listenFds;
}


int
handleFdSetSignal(Socket signalFd, const std::set<Socket> &listenFds, std::set<Socket> &readFds, fd_set *readset, fd_set *master_set)
{
	auto maxFd = 0;
	if (FD_ISSET(signalFd, readset)) {
		Log("Signal");
		
		auto s = native::Socket{ INVALID_SOCKET };
		char buf[128];
		auto result = ::recv(signalFd, buf, sizeof(buf), 0);
		if (result > 0)
		{
			Log("Signal: received : ", result, " bytes. ");;
			auto nFds = result / sizeof(native::Socket);
			native::Socket* ptr = (native::Socket*)buf;
			for (auto i = 0; i < nFds; ++i)
			{
				auto fd = ptr[i];
				Log("Signal: Add: ", ptr[i]);
				maxFd = std::max(maxFd, (int)ptr[i]);
				if (listenFds.find(fd) == std::end(listenFds))
				{
					readFds.emplace(fd);
				}
				FD_SET(fd, master_set);
			}
		}
	}
	return maxFd;
}

void
handleIncomingConnections(thread_pool &pool_, Socket sendFd, const std::set<Socket> &listenFds, fd_set *readset, fd_set *master_set)
{
	for (auto fd : listenFds)
	{
		if (FD_ISSET(fd, readset))
		{
			FD_CLR(fd, master_set);
			Log("main: clr: ", fd);
			pool_.enqueueWork([sendFd, fd](thread_pool &) {

				native::Socket fds[2];
				fds[0] = fd;
				fds[1] = accept(fd, nullptr, nullptr);
				auto sendSize = (fds[1] != INVALID_SOCKET) ? 2 : 1;
				if (sendSize == 2)
					Log("accept: Sending: ", fds[0], ", ", fds[1]);
				else
					Log("accept: Sending: ", fds[0]);;

				auto result = ::send(sendFd, (char *)fds, (int)sizeof(native::Socket)*sendSize, 0);
				Log("accept: Sent: ", result, " bytes ");
			});
		}
	}
}

void
handleIncomingData(thread_pool &pool_, Socket sendFd, std::set<Socket> &readFds, fd_set *readset, fd_set *master_set)
{
	for (auto fd : readFds)
	{
		if (FD_ISSET(fd, readset))
		{
			FD_CLR(fd, master_set);
			Log("readReady: clr: ", fd);
			pool_.enqueueWork([=](thread_pool &p) {
				std::shared_ptr<char> buf{ new char[256] };;
				auto result = ::recv(fd, buf.get(), 256, 0);
				Log("readReady: recv: ", fd);
				if (result > 0)
				{
					p.enqueueWork([=](thread_pool &p) {
						Log("readReady: send: ", fd);
						::send(fd, buf.get(), result, 0);
					});
					Log("readReady: signal: ", fd);
					auto result = ::send(sendFd, (char *)&fd, (int)sizeof(fd), 0);
					Log("readReady: Sent: ", result, " bytes ");
				}
				else
				{
					if (result == 0)
					{
						// client close
						Log("connection closed by client");
					}
					else if (result < 0)
					{
						Log("Error reading from client");
					}
				}
			});
		}
	}
}


static
std::array<native::Socket, 2>
tpipe()
{
	std::array<native::Socket, 2> p{INVALID_SOCKET,INVALID_SOCKET};
	// make the server socket
	auto listernerFd = ::socket(AF_INET, SOCK_STREAM, 0);
 	struct sockaddr_in loopback;
	std::memset(&loopback, '\0', sizeof(loopback));
	struct sockaddr *serverptr = (struct sockaddr*)&loopback;
	int loopback_size = (int)sizeof(loopback);

	loopback.sin_family = AF_INET;
	loopback.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	loopback.sin_port = 0;

	/* Initiate connection */
	if (bind(listernerFd, serverptr, sizeof(loopback))<0)
		error("bind");

	getsockname(listernerFd, (struct sockaddr *)&loopback, &loopback_size);

	listen(listernerFd, 1);

	p[PSEND] = ::socket(AF_INET, SOCK_STREAM, 0);
	connect(p[PSEND], (struct sockaddr *)&loopback, loopback_size);

	p[PRECV] = accept(listernerFd, 0, 0);

	native::closesocket(listernerFd);
	return p;
}
