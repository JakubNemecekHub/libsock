#include <print>

#include "sock/sock.hpp"


int main()
{

    Conf conf {
        .addr_str = "localhost",
        .port_str = "8888",
        .data = { 10, 12, 14 },
    };
    const auto res { send_udp(conf) };
    if ( res.has_value() )
    {
        std::println("No error, got {} bytes", res.value().size());
    }
    else
    {
        std::println("Error");
    }

    return 0;
}