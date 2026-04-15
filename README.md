# libsock

Primitive multiplatform static library for TCP and UDP functionality. 🧦

## General and off topic notes

### Sources of symbols

| What           | Windows | Linux         |
|----------------|---------|---------------|
| addrinfo       | ?       | netdb.h       |
| AF_INET        | ?       | socket.h      |
| SOCK_STREAM    | ?       | socket_type.h |
| IPPROTO_TCP    | ?       | in.h          |
| INVALID_SOCKET | ?       | nowhere       |
| SOCKET_ERROR   | ?       | nowhere       |
| getnameinfo    | ?       | netdb.h       |
| NI_MAXHOST     | ?       | netdb.h       |
| NI_MAXSERV     | ?       | netdb.h       |
| NI_NUMERICHOST | ?       | netdb.h       |
| NI_NUMERICSERV | ?       | netdb.h       |
| socket         | ?       | socket.h      |
| bind           | ?       | socket.h      |
| close          | ?       | unistd.h      |
| accept         | ?       | socket.h      |
| recv           | ?       | socket2.h     |
| send           | ?       | socket.h      |
| strlen         | ?       | string.h      |

### Creating IPv4 and IPv6 addresses

Originaly I used `getaddrinfo` to create address and port in their proper types. This seemed to open additional sockets. After further examination, I discovered, that these extra socket are created right at the beginning of my code, somewhere in one of the libraries I'm using. They are probably ment to be used for DNS calls. Is there a way to disable it?

Note that `getaddrinfo` will use a DNS if wi input domain name and not IP address.

So I manually created `sockaddr_in` struct with IPv4 address. ("in" seems to mean "information", it's not a preposition.) The `sockaddr_in` struct must be cast to the generic `sockaddr` struct which is expected by the `socket` and `bind` functions. These function also expect the byte-length of the addresss; here we cannot pass the length of our `sockaddr` but we must use the actual `sockaddr_in` struct. I don't properly understand this.

```cpp
sockaddr_in addr_in {
    .sin_family = AF_INET,
    .sin_port = htons(port),
};
if ( inet_pton(AF_INET, conf.addr_str.c_str(), &addr_in.sin_addr) <= 0 )
{
    std::println(stderr, "invalid address or address not supported: {}", conf.addr_str);
    return;
}
const sockaddr *addr { reinterpret_cast<const sockaddr*>(&addr_in) };
const auto addr_size { sizeof(addr_in) };
```

IPv6 address is created in a similar way.

```cpp
sockaddr_in6 addr_in6 {
    .sin6_family = AF_INET6,
    .sin6_port = htons(port),
};
if ( inet_pton(AF_INET6, conf.addr_str.c_str(), &addr_in6.sin6_addr) <= 0 )
{
    std::println(stderr, "invalid address or address not supported: {}", conf.addr_str);
    return;
}
const sockaddr *addr { reinterpret_cast<const sockaddr*>(&addr_in6) };
const auto addr_size { sizeof(addr_in6) };
```

Again, the `sockaddr_in6` struct is cast into `sockaddr` pointer; the byte-length of he address is the size of the `sockaddr_in6`.

The `getaddrinfo` allows us to create both IPv4 and IPv6 addresses based on the value of the `ai_family` field. `AF_INET` for IPv4, `AF_INET6` for IPv6 and `AF_UNSPEC` will create IP address of proper version based on input. How does it evaluate domain names and "localhost"? Does it default to IPv6? Because I dont understand this, Im using the following approach to switch between IPv4 and IPv6 based on program arguments: `.ai_family = conf.ipv4 ? AF_INET : AF_INET6`.

In the tcp_client I use `Af_UNSPEC` and it seems to translate "localhost" into "::1".

## TO DO

- Test on Windows.
- In header.hpp move all the aliasing to the Windows section. I want the code to be Linux compatible. Is that possible?
- Handle UDP client's call to closed port. Now it just gets stuck. Set time-out?
- What should we do if getaddrinfo returns multiple addresses? In our ntp clietn we would like to send request to all addresses.
- Add multithreading for servers.
