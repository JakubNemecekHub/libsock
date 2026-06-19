#include <print>

#include "sock/sock.hpp"
#include "header.hpp"

namespace sck
{

std::optional<std::vector<char>> send_udp(Conf conf)
{
    #ifdef _WIN32
		WSADATA wsa_data;
		const int indicator_startup { WSAStartup(MAKEWORD(2, 2), &wsa_data) };
		if ( indicator_startup != 0 )
		{
			std::println(stderr, "WSAStartup failed with error: {}", indicator_startup);
			return std::nullopt;
		}
	#endif

    addrinfo *result { nullptr };
    const addrinfo hints {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
        .ai_protocol = IPPROTO_UDP,
    };

    const int indicator_address { getaddrinfo(conf.addr_str.c_str(), conf.port_str.c_str(), &hints, &result) };
    if ( indicator_address != 0 )
	{
        std::println("getaddrinfo failed, error with port {} {}", conf.port_str, gai_strerror(indicator_address));
		freeaddrinfo(result);
        clean_up();
		return std::nullopt;
	}

    const SOCKET udp_socket { socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
    if ( udp_socket == INVALID_SOCKET )
	{
        std::println(stderr, "socket failed with error: {}", get_last_error());
		freeaddrinfo(result);
		clean_up();
		return std::nullopt;
	}

    const ssize_t indicator_send { sendto(udp_socket, conf.data.data(), conf.data.size(), 0, result->ai_addr, (int)result->ai_addrlen) };
    if ( indicator_send == SOCKET_ERROR )
    {
        std::println(stderr, "sendto failed with error: {}", error_message());
        closesocket(udp_socket);
        clean_up();
        return std::nullopt;
    }
    
    freeaddrinfo(result);

    constexpr int DEFAULT_BUFLEN { 65536 };
    std::vector<char> recvbuf ( DEFAULT_BUFLEN );
    sockaddr_storage from_address {};
    socklen_t from_address_length { sizeof(from_address) };
    const ssize_t indicator_recv { recvfrom(udp_socket, recvbuf.data(), recvbuf.size(), 0,
		                                    reinterpret_cast<sockaddr*>(&from_address), &from_address_length) };
    if ( indicator_recv < 0 )
    {
        std::println(stderr, "recvfrom failed with error: {}", get_last_error());
        return std::nullopt;
    }

    closesocket(udp_socket);
    clean_up();

    recvbuf.resize(indicator_recv);
    return recvbuf;
}


std::optional<std::vector<char>> send_tcp(Conf conf)
{
    #ifdef _WIN32
		WSADATA wsa_data;
		const int indicator_startup { WSAStartup(MAKEWORD(2, 2), &wsa_data) };
		if ( indicator_startup != 0 )
		{
			std::println(stderr, "WSAStartup failed with error: {}", indicator_startup);
			return std::nullopt;
		}
	#endif

    addrinfo *result { nullptr };
    const addrinfo hints {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
    };

    const int indicator_address { getaddrinfo(conf.addr_str.c_str(), conf.port_str.c_str(), &hints, &result) };
    if ( indicator_address != 0 )
	{
        std::println("Error with port {} {}", conf.port_str, gai_strerror(indicator_address));
		freeaddrinfo(result);
        clean_up();
		return std::nullopt;
	}

    const SOCKET client_socket { socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
    if ( client_socket == INVALID_SOCKET )
	{
        std::println(stderr, "socket failed with error: {}", get_last_error());
		freeaddrinfo(result);
		clean_up();
		return std::nullopt;
	}

    char ip_buffer[NI_MAXHOST];
	char port_buffer[NI_MAXSERV];
	const int indicator { getnameinfo(result->ai_addr, result->ai_addrlen,
	                                  ip_buffer, sizeof(ip_buffer),
									  port_buffer, sizeof(port_buffer),
									  NI_NUMERICHOST | NI_NUMERICSERV) };
	if ( indicator != 0 )
	{
        std::println(stderr, "cannot retrieve address");
	}

    const int indicator_connect { connect(client_socket, result->ai_addr, (int)result->ai_addrlen) };
    if ( indicator_connect == SOCKET_ERROR )
    {
        std::println("error while connecting to {}:{} {}", ip_buffer, port_buffer, error_message());
		freeaddrinfo(result);
		clean_up();
		return std::nullopt;
    }
    // Here we should try the next address in result, it might have a pointer to another addrinfo object
    // For this we would use the ptr variable that, in the tutorial, was declared together with result

    freeaddrinfo(result);

    const ssize_t indicator_send { send(client_socket, conf.data.data(), conf.data.size(), 0) };
    if ( indicator_send == SOCKET_ERROR )
    {
        std::println(stderr, "send failed with error: {}", get_last_error());
        closesocket(client_socket);
        clean_up();
        return std::nullopt;
    }

    const int indicator_shutdown { shutdown(client_socket, SHUT_WR) };
    if ( indicator_shutdown == SOCKET_ERROR )
    {
        std::println(stderr, "shutdown failed with error: {}", error_message());
        closesocket(client_socket);
        clean_up();
        return std::nullopt;
    }

    constexpr int DEFAULT_BUFLEN { 512 };
    std::vector<char> recvbuf ( DEFAULT_BUFLEN );
    ssize_t indicator_recv;
    ssize_t total_recv { 0 };
    do
    {
        indicator_recv = recv(client_socket, recvbuf.data(), recvbuf.size(), 0);
        if ( indicator_recv > 0 )
        {
            total_recv += indicator_recv;

        }
        else if ( indicator_recv < 0 )
        {
            std::println(stderr, "recv failed with error: {}", get_last_error());
            return std::nullopt;
        }
    } while ( indicator_recv > 0 );

    closesocket(client_socket);
    clean_up();

    recvbuf.resize(total_recv);
    return recvbuf;
}


void listen_udp(Conf conf)
{
	#ifdef _WIN32
		WSADATA wsa_data;
		const int indicator_startup { WSAStartup(MAKEWORD(2, 2), &wsa_data) };
		if ( indicator_startup != 0 )
		{
			std::println(stderr, "WSAStartup failed with error: {}", indicator_startup);
			return;
		}
	#endif

	addrinfo *result { nullptr };
	const addrinfo hints {
		.ai_flags = 0,
		.ai_family = conf.ipv4 ? AF_INET : AF_INET6,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = IPPROTO_UDP,
	};

	const int indicator_address { getaddrinfo(conf.addr_str.c_str(), conf.port_str.c_str(), &hints, &result) };
	if ( indicator_address != 0 )
	{
		std::println("error with port {} {}", conf.port_str, gai_strerror(indicator_address));
		freeaddrinfo(result);
		clean_up();
		return;
	}

	char ip_buffer[NI_MAXHOST];
	char port_buffer[NI_MAXSERV];
	const int extraction_indicator { getnameinfo(result->ai_addr, result->ai_addrlen,
	                                             ip_buffer, sizeof(ip_buffer),
									             port_buffer, sizeof(port_buffer),
									             NI_NUMERICHOST | NI_NUMERICSERV) };

	const SOCKET udp_socket { socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
	if ( udp_socket == INVALID_SOCKET )
	{
		std::println(stderr, "error at socket(): {}", get_last_error());
		delete result;
		clean_up();
		return;
	}

	const int indicator_bind { bind(udp_socket, result->ai_addr, (int)result->ai_addrlen) };
	if ( indicator_bind == SOCKET_ERROR )
	{
		std::println("error binding to {}:{} {}", ip_buffer, port_buffer, error_message());
		closesocket(udp_socket);
		clean_up();
		return;
	}
	freeaddrinfo(result);

	if ( extraction_indicator == 0 )
	{
		std::println("UDP server bound to {}:{}", ip_buffer, port_buffer);
	}
	else
	{
		std::println(stderr, "I'm bound to something but don't know what. {}", error_message());
	}

    constexpr size_t DEFAULT_BUFLEN { 65536 };
	char recvbuf[DEFAULT_BUFLEN];
	const char *sendbuf { "this is an answer" };
	sockaddr_storage from_address {};
	socklen_t from_address_length { sizeof(from_address) };

	while ( true )
	{

		from_address_length = sizeof(from_address);
		const ssize_t indicator_recv { recvfrom(udp_socket, recvbuf, sizeof(recvbuf), 0,
											    reinterpret_cast<sockaddr*>(&from_address), &from_address_length) };
		if ( indicator_recv > 0 )
		{
			std::println("bytes received: {}", indicator_recv);
			std::println("payload: {}", std::string_view(recvbuf, indicator_recv));
			const ssize_t indicator_send { sendto(udp_socket, sendbuf, (int)strlen(sendbuf), 0, 
				                                  reinterpret_cast<sockaddr*>(&from_address), from_address_length) };
			if ( indicator_send == SOCKET_ERROR )
			{
				std::println(stderr, "sendto failed with error: {}", get_last_error());
				clean_up();
				return;
			}
			std::println("Bytes send: {}", indicator_send);
		}
		else if ( indicator_recv == 0 )
		{
			std::println("connection interupted...");
		}
		else
		{
			std::println(stderr, "recvfrom failed with error: {}", get_last_error());
			clean_up();
			return;
		}

	}

	closesocket(udp_socket);
	clean_up();
}


void listen_tcp(Conf conf)
{

    #ifdef _WIN32
		WSADATA wsa_data;
		const int indicator_startup { WSAStartup(MAKEWORD(2, 2), &wsa_data) };
		if ( indicator_startup != 0 )
		{
			std::println(stderr, "WSAStartup failed with error: {}", indicator_startup);
			return;
		}
	#endif

	addrinfo *result { nullptr };
	const addrinfo hints {
		.ai_family = conf.ipv4 ? AF_INET : AF_INET6,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP,
	};
	const int indicator_address { getaddrinfo(conf.addr_str.c_str(), conf.port_str.c_str(), &hints, &result) };
	if ( indicator_address != 0 )
	{
        std::println("getaddrinfo failed, error with port {} {}", conf.port_str, gai_strerror(indicator_address));
		freeaddrinfo(result);
        clean_up();
		return;
	}

	char ip_buffer[NI_MAXHOST];
	char port_buffer[NI_MAXSERV];
	const int extraction_indicator { getnameinfo(result->ai_addr, result->ai_addrlen,
	                                             ip_buffer, sizeof(ip_buffer),
									             port_buffer, sizeof(port_buffer),
									             NI_NUMERICHOST | NI_NUMERICSERV) };

	const SOCKET listen_socket { socket(result->ai_family, result->ai_socktype, result->ai_protocol) };
	if ( listen_socket == INVALID_SOCKET )
	{
		std::println(stderr, "socket failed with error: {}", get_last_error());
		clean_up();
		return;
	}

	const int indicator_bind { bind(listen_socket, result->ai_addr, result->ai_addrlen) };
	if ( indicator_bind == SOCKET_ERROR )
	{
		std::println("error binding to {}:{} {}", conf.addr_str, conf.port_str, error_message());
		closesocket(listen_socket);
		clean_up();
		return;
	}
	freeaddrinfo(result);

	const int indicator_listening { listen(listen_socket, SOMAXCONN) };
	if ( indicator_listening == SOCKET_ERROR )
	{
		std::println(stderr, "listen failed with error: {}", get_last_error());
		closesocket(listen_socket);
		clean_up();
		return;
	}

	if ( extraction_indicator == 0 )
	{
		std::println("TCP server listening at {}:{}", ip_buffer, port_buffer);
	}
	else
	{
		std::println(stderr, "I'm listening but don't know where. {}", error_message());
	}

    constexpr size_t DEFAULT_BUFLEN { 512 };
	char recvbuf[DEFAULT_BUFLEN];
	const char *sendbuf { "this is an answer" };

	while ( true )
	{

		const SOCKET client_socket { accept(listen_socket, nullptr, nullptr) };
		if ( client_socket == INVALID_SOCKET )
		{
			std::println(stderr, "accept failed with error: {}", get_last_error());
			std::println("waiting for connection...");
		}

		ssize_t indicator_recv;
		size_t  packet_id { 0 };
		do
		{
			indicator_recv = recv(client_socket, recvbuf, sizeof(recvbuf), 0) ;
			if ( indicator_recv > 0 )
			{
				// TO DO: processs the data
				++packet_id;
			}
			else if ( indicator_recv < 0 )
			{
				std::println(stderr, "recv failed with error: {}", get_last_error());
				closesocket(client_socket);
				clean_up();
				return;
			}
		} while ( indicator_recv > 0 );
		std::println("got {} packets", packet_id);

		const ssize_t indicator_send { send(client_socket, sendbuf, strlen(sendbuf), 0) };
		if ( indicator_send == SOCKET_ERROR )
		{
			std::println(stderr, "send failed with error: {}", error_message());
			closesocket(client_socket);
			clean_up();
			return;
		}

		std::println("client disconnected");
		closesocket(client_socket);

	}

	closesocket(listen_socket);
	clean_up();
}

} // namespace sck
