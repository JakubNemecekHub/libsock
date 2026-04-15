#pragma once

#include <system_error>
#include <cstring>
#include <errno.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	int get_last_error() { return WSAGetLastError(); }  // Return result of WSAGetLastError()
	#define SHUT_WR SD_SEND	   // Different approach on Linux
#else
	#include <sys/socket.h>  // socket, binf, sendto, recvfrom
	#include <netinet/in.h>  // sockaddr_in, IPROTO-UDP and similar
	#include <arpa/inet.h>   // htonl
	#include <netdb.h>       // addrinfo, getaddrinfo, also used for DNS lookup
	#include <unistd.h>      // close
	#include <errno.h>
	#include <string.h>		 // strlen
	#define SOCKET int       // On Linux socket is just an int
	#define INVALID_SOCKET -1  // Not present on Linux
    #define SOCKET_ERROR   -1  // Not present on Linux
	// #define SD_SEND SHUT_WR	   // Different approach on Linux - Pending move to Windows
	int get_last_error () { return errno; }  // Return errno
	#define closesocket close
#endif

std::string error_message()
{
	#ifdef _WIN32
		return std::system_category().message(WSAGetLastError());
	#else
		return std::system_category().message(errno); 
	#endif
}

void clean_up()
{
	#ifdef _WIN32
		WSACleanup();
	#endif
}