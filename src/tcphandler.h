#pragma once

#include <iostream>
#include <functional>

#include <boost/optional.hpp>

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
        HttpContext()
        {
            reset();
        }

        void reset()
        {
            requestParser_.emplace();
            responseParser_.emplace();
        }

        boost::optional<HttpRequestParser> requestParser_;
        boost::optional<HttpResponseParser> responseParser_;
    };
public:
    TcpHandler(HttpHandlerPtr h)
        : handler_(h)
    {
    }

    template<typename T>
    inline void log(std::chrono::nanoseconds timestamp, const Tuple4 & tuple4, const char * event, const T & message)
    {
        std::cout << "[" << timestamp.count() << "]"
                  << "[" << tuple4 << "]"
                  << "[" << event << "]"
                  << message << std::endl;
    }

    inline void log(std::chrono::nanoseconds timestamp, const Tuple4 & tuple4, const char * event)
    {
        log(timestamp, tuple4, event, "");
    }

    void handleTcpEstablished(Tins::TCPIP::Stream & stream)
    {
        stream.client_data_callback(std::bind(&TcpHandler::handleData, this, std::placeholders::_1));
        stream.server_data_callback(std::bind(&TcpHandler::handleData, this, std::placeholders::_1));
        stream.stream_closed_callback(std::bind(&TcpHandler::handleTcpClosed, this, std::placeholders::_1));

        Tuple4 tuple4 = getTuple4FromStream(stream);
        log(stream.create_time(), tuple4, "ESTABLISHED");
        auto & c = contexts_[tuple4];
        c.reset();
        stream.create_time();

        //stream.auto_cleanup_payloads(false);
    }

    void handleTcpClosed(Tins::TCPIP::Stream & stream)
    {
        Tuple4 tuple4 = getTuple4FromStream(stream);
        auto iter = contexts_.find(tuple4);
        if(iter == contexts_.end())
            return;

        if(stream.client_flow().is_finished())
        {
            //if(iter->second->requestParser_->got_some())
            //{
            //    boost::beast::error_code ec;
            //    iter->second->requestParser_->put_eof(ec);
            //    if(!ec && iter->second->requestParser_->is_done())
            //    {
            //        handleHttpRequest(stream.last_seen(), tuple4, iter->second->requestParser_);
            //    }
            //}
            log(stream.create_time(), tuple4, "CLIENT_CLOSED");
        }

        if(stream.server_flow().is_finished())
        {
            // no content-length in header
            if(iter->second.responseParser_->got_some())
            {
                boost::beast::error_code ec;
                iter->second.responseParser_->put_eof(ec);
                if(!ec && iter->second.responseParser_->is_done())
                {
                    handleHttpResponse(stream.last_seen(), tuple4, iter->second.responseParser_);
                }
            }
            log(stream.create_time(), tuple4, "SERVER_CLOSED");
        }

        if(stream.is_finished())
        {
            log(stream.create_time(), tuple4, "CLOSED");
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
            used += iter->second.requestParser_->put(boost::asio::const_buffer(stream.client_payload().data() + used, stream.client_payload().size() - used), ec);
            if(!ec)
            {
                if(iter->second.requestParser_->is_done())
                {
                    handleHttpRequest(stream.last_seen(), tuple4, iter->second.requestParser_);
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
            used += iter->second.responseParser_->put(boost::asio::const_buffer(stream.server_payload().data() + used, stream.server_payload().size() - used), ec);

            if(!ec)
            {
                if(iter->second.responseParser_->is_done())
                {
                    handleHttpResponse(stream.last_seen(), tuple4, iter->second.responseParser_);
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

        return;

    parse_error:
        stream.client_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
        stream.server_data_callback(Tins::TCPIP::Stream::stream_callback_type{});
        stream.stream_closed_callback(Tins::TCPIP::Stream::stream_callback_type{});
        log(std::chrono::system_clock::now().time_since_epoch(), tuple4, "DROP");
        contexts_.erase(tuple4);
    }

    void handleHttpRequest(std::chrono::nanoseconds timestamp, const Tuple4 & tuple4, boost::optional<HttpRequestParser> & parser)
    {
        handler_->handleHttpRequest(tuple4, parser->get());
        log(timestamp, tuple4, "REQUEST", parser->get());
        parser.emplace();
    }

    void handleHttpResponse(std::chrono::nanoseconds timestamp, const Tuple4 & tuple4, boost::optional<HttpResponseParser> & parser)
    {
        handler_->handleHttpResponse(tuple4, parser->get());
        log(timestamp, tuple4, "RESPONSE", parser->get());
        parser.emplace();
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

    std::map<Tuple4, HttpContext> contexts_;
};
