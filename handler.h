#pragma once

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <functional>

#include <boost/asio.hpp>

#include <tins/tcp_ip/stream.h>
#include <tins/ip_address.h>
#include <tins/ipv6_address.h>

#include "http.h"

using endpoint = boost::asio::ip::tcp::endpoint;
using address = boost::asio::ip::address;

struct Tuple4
{
    endpoint client;
    endpoint server;

    static Tuple4 fromStream(const Tins::TCPIP::Stream & stream)
    {
        return Tuple4
        {
            { address::from_string(stream.is_v6() ? stream.client_addr_v6().to_string() : stream.client_addr_v4().to_string()), stream.client_port() },
            { address::from_string(stream.is_v6() ? stream.server_addr_v6().to_string() : stream.server_addr_v4().to_string()), stream.server_port() }
        };
    }

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

class Handler
{
    struct HttpParserPair
    {
        Tuple4 tuple4_;
        std::unique_ptr<HttpParser> clientParser_;
        std::unique_ptr<HttpParser> serverParser_;
    };
public:
    void handleTcpEstablished(Tins::TCPIP::Stream & stream)
    {
        stream.client_data_callback(std::bind(&Handler::handleData, this, std::placeholders::_1));
        stream.server_data_callback(std::bind(&Handler::handleData, this, std::placeholders::_1));
        stream.stream_closed_callback(std::bind(&Handler::handleTcpClosed, this, std::placeholders::_1));

        Tuple4 tuple4 = Tuple4::fromStream(stream);
        std::cout << tuple4 << "[ESTABLISHED]" << std::endl;
        auto & parser = parsers_[tuple4];
        parser.tuple4_ = tuple4;
        parser.clientParser_.reset(new HttpParser{});
        parser.serverParser_.reset(new HttpParser{});

        parser.clientParser_->setMessageHandler([this, &parser](HttpMessage & message)
        {
            handleHttpMessage(parser.tuple4_, true, message);
        });

        parser.serverParser_->setMessageHandler([this, &parser](HttpMessage & message)
        {
            handleHttpMessage(parser.tuple4_, false, message);
        });

        //stream.auto_cleanup_payloads(false);
    }

    void handleTcpClosed(Tins::TCPIP::Stream & stream)
    {
        Tuple4 tuple4 = Tuple4::fromStream(stream);
        auto iter = parsers_.find(tuple4);
        if(iter == parsers_.end())
            return;

        if(stream.client_flow().is_finished())
        {
            std::cout << tuple4 << "[CLIENT_CLOSED]" << std::endl;
        }

        if(stream.server_flow().is_finished())
        {
            std::cout << tuple4 << "[SERVER_CLOSED]" << std::endl;
        }

        if(stream.is_finished())
        {
            std::cout << tuple4 << "[CLOSED]" << std::endl;
            parsers_.erase(iter);
        }
    }

    void handleData(Tins::TCPIP::Stream & stream)
    {
        Tuple4 tuple4 = Tuple4::fromStream(stream);
        auto iter = parsers_.find(tuple4);
        if(iter == parsers_.end())
            return;

        auto & clientPayload = stream.client_payload();
        auto & serverPayload = stream.server_payload();
        bool parseFailed = false;

        if(clientPayload.size() > 0)
        {
            size_t result = iter->second.clientParser_->parse((const char *)clientPayload.data(), clientPayload.size());
            if(result != clientPayload.size())
            {
                parseFailed = true;
            }
        }

        if(serverPayload.size() > 0)
        {
            size_t result = iter->second.serverParser_->parse((const char *)serverPayload.data(), serverPayload.size());
            if(result != serverPayload.size())
            {
                parseFailed = true;
            }
        }

        if(parseFailed)
        {
            stream.client_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
            stream.server_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
            stream.stream_closed_callback(Tins::TCPIP::Stream::stream_callback_type{});
            std::cout << tuple4 << "[DROP]" << std::endl;
            parsers_.erase(tuple4);
        }
    }

    /*
     * @direction@ true: client->server, false: server->client
     */
    void handleHttpMessage(const Tuple4 & tuple4, bool direction, HttpMessage & message)
    {
        std::cout << tuple4 << (direction ? "[CLIENT_DATA]" : "[SERVER_DATA]")  << message.toString() << std::endl;
    }

    std::map<Tuple4, HttpParserPair> parsers_;
};
