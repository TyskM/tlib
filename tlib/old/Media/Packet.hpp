#pragma once

// TODO: Remove sfml dependency
#include <SFML/Network/Packet.hpp>

// << Writes, and >> Reads, don't forget zzzz

using SFPacket = sf::Packet;

// std::vector
template <typename T>
SFPacket& operator<<(SFPacket& p, const std::vector<T>& v)
{
    p << static_cast<uint32_t>(v.size());
    for (auto& item : v) { p << item; }
    return p;
}

// std::vector
template <typename T>
SFPacket& operator>>(SFPacket& p, std::vector<T>& v)
{
    uint32_t size;
    p >> size;
    v.clear();
    v.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        T next;
        p >> next;
        v.push_back(next);
    }
    return p;
}