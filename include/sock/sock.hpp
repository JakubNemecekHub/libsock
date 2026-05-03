#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace sck
{

struct Conf
{
    std::string addr_str;
    std::string port_str;
    bool ipv4 { false };
    std::vector<char> data;
};


std::optional<std::vector<char>> send_udp(Conf conf);
std::optional<std::vector<char>> send_tcp(Conf conf);
void listen_udp(Conf conf);
void listen_tcp(Conf conf);

} // namespace sck
