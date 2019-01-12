#pragma once

#include <iostream>
#include <functional>

#include <tins/tcp_ip/stream.h>
#include <tins/ip_address.h>
#include <tins/ipv6_address.h>

#include "http.h"
#include "tuple4.h"
#include "httphandler.h"

class TcpHandler
{
    struct HttpContext
    {
        HttpRequestParser requestParser_;
        HttpResponseParser responseParser_;
    };
public:
    TcpHandler(HttpHandlerPtr h)
        : handler_(h)
    {
    }

    void handleTcpEstablished(Tins::TCPIP::Stream & stream)
    {
        stream.client_data_callback(std::bind(&TcpHandler::handleData, this, std::placeholders::_1));
        stream.server_data_callback(std::bind(&TcpHandler::handleData, this, std::placeholders::_1));
        stream.stream_closed_callback(std::bind(&TcpHandler::handleTcpClosed, this, std::placeholders::_1));

        Tuple4 tuple4 = getTuple4FromStream(stream);
        std::cout << tuple4 << "[ESTABLISHED]" << std::endl;
        auto & c = contexts_[tuple4];
        c = std::make_shared<HttpContext>();

        //stream.auto_cleanup_payloads(false);
    }

    void handleTcpClosed(Tins::TCPIP::Stream & stream)
    {
        Tuple4 tuple4 = getTuple4FromStream(stream);
        auto iter = contexts_.find(tuple4);
        if(iter == contexts_.end())
            return;

        // todo: process http/1.0

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
            contexts_.erase(iter);
        }
    }

    void handleData(Tins::TCPIP::Stream & stream)
    {
        Tuple4 tuple4 = getTuple4FromStream(stream);
        auto iter = contexts_.find(tuple4);
        if(iter == contexts_.end())
            return;

        boost::beast::error_code ec;
        std::size_t used = 0;
        for (;stream.client_payload().size() > used;)
        {
            used += iter->second->requestParser_.put(boost::asio::const_buffer(stream.client_payload().data() + used, stream.client_payload().size() - used), ec);
            if(!ec)
            {
                if(iter->second->requestParser_.is_done())
                {
                    handleHttpRequest(tuple4, iter->second->requestParser_.get());
                }
            }
            else if(ec == boost::beast::http::error::need_more)
            {
                break;
            }
            else
            {
                goto parse_error;
            }
        }

        used = 0;
        for (;stream.server_payload().size() > used;)
        {
            used += iter->second->responseParser_.put(boost::asio::const_buffer(stream.server_payload().data() + used, stream.server_payload().size() - used), ec);

            if(!ec)
            {
                //assert(iter->second->responseParser_.is_done());
                handleHttpResponse(tuple4, iter->second->responseParser_.get());
            }
            else if(ec == boost::beast::http::error::need_more)
            {
                break;
            }
            else
            {
                goto parse_error;
            }
        }

        return;

    parse_error:
        stream.client_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
        stream.server_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
        stream.stream_closed_callback(Tins::TCPIP::Stream::stream_callback_type{});
        std::cout << tuple4 << "[DROP]" << std::endl;
        contexts_.erase(tuple4);
    }

    void handleHttpRequest(const Tuple4 & tuple4, HttpRequest & message)
    {
        handler_->handleHttpRequest(tuple4, message);
        std::cout << tuple4 << "[REQUEST]" << message << std::endl;
    }

    void handleHttpResponse(const Tuple4 & tuple4, HttpResponse & message)
    {
        handler_->handleHttpResponse(tuple4, message);
        std::cout << tuple4 << "[RESPONSE]" << message << std::endl;
    }

    Tuple4 getTuple4FromStream(const Tins::TCPIP::Stream & stream)
    {
        return Tuple4
        {
            { Address::from_string(stream.is_v6() ? stream.client_addr_v6().to_string() : stream.client_addr_v4().to_string()), stream.client_port() },
            { Address::from_string(stream.is_v6() ? stream.server_addr_v6().to_string() : stream.server_addr_v4().to_string()), stream.server_port() }
        };
    }

private:
    HttpHandlerPtr handler_;

    std::map<Tuple4, std::shared_ptr<HttpContext> > contexts_;
};
