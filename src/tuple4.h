#pragma once

#include <iosfwd>

#include "http.h"

struct Tuple4
{
    Endpoint client;
    Endpoint server;

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
