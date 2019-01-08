#pragma once

#include <iostream>

#include <boost/asio.hpp>

struct Tuple4
{
    boost::asio::ip::tcp::endpoint client;
    boost::asio::ip::tcp::endpoint server;

    bool operator < (const Tuple4 & other) const
    {
        if(client == other.client)
        {
            return server < other.server;
        }
        return client < other.client;
    }
};

inline std::ostream & operator<< (std::ostream & os, const Tuple4 & tuple4)
{
    os << "[" << tuple4.client.address().to_string() << ":" << tuple4.client.port()
       << " -> " << tuple4.server.address().to_string() << ":" << tuple4.server.port() << "]";
    return os;
}
