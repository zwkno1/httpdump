#pragma once

#include <boost/noncopyable.hpp>

#include "detail/http_parser.h"

class HttpParser : private boost::noncopyable
{
public:
    HttpParser()
    {
        init();
    }

    template<typename Handler>
    void setMessageHandler(Handler && handler)
    {
        messageHandler_ = std::forward<Handler>(handler);
    }

    size_t parse(const char * data, size_t size)
    {
        size_t parsed = http_parser_execute(&parser_, &settings_, data, size);
        return parsed;
    }

    void handleMessageComplete()
    {
        message_.isRequest = (parser_.type == HTTP_REQUEST);
        message_.version.major = parser_.http_major;
        message_.version.minor = parser_.http_minor;
        message_.method = parser_.method;
        message_.statusCode = parser_.status_code;

        messageHandler_(message_);
        clear();
    }

    int appendUrl(const char * buf, size_t len)
    {
        message_.url.append(buf, len);
        return 0;
    }

    int appendBody(const char * buf, size_t len)
    {
        message_.body.append(buf, len);
        return 0;
    }

    int appendHeaderField(const char * buf, size_t len)
    {
        checkAppendHeader();
        tmpField_.append(buf, len);
        return 0;
    }

    int appendHeaderValue(const char * buf, size_t len)
    {
        shouldAppendHeader_ = true;
        tmpValue_.append(buf, len);
        return 0;
    }

    void handleHeadersComplete()
    {
        checkAppendHeader();
    }

    static int urlCallback(http_parser *p, const char *buf, size_t len)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->appendUrl(buf, len);
        return 0;
    }

    static int bodyCallback(http_parser *p, const char *buf, size_t len)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->appendBody(buf, len);
        return 0;
    }

    static int headerFieldCallback(http_parser *p, const char *buf, size_t len)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->appendHeaderField(buf, len);
        return 0;
    }

    static int headerValueCallback(http_parser *p, const char *buf, size_t len)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->appendHeaderValue(buf, len);
        return 0;
    }

    static int headersCompleteCallback(http_parser *p)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->handleHeadersComplete();
        return 0;
    }

    static int messageCompleteCallback(http_parser *p)
    {
        auto parser = static_cast<HttpParser *>(p->data);
        parser->handleMessageComplete();
        return 0;
    }

private:
    void init()
    {
        std::memset(&settings_, 0, sizeof(settings_));
        settings_.on_url = &HttpParser::urlCallback;
        settings_.on_body = &HttpParser::bodyCallback;
        settings_.on_header_field = &HttpParser::headerFieldCallback;
        settings_.on_header_value = &HttpParser::headerValueCallback;
        settings_.on_headers_complete = &HttpParser::headersCompleteCallback;
        settings_.on_message_complete = &HttpParser::messageCompleteCallback;
        parser_.data = this;
        http_parser_init(&parser_, HTTP_BOTH);
        clear();
    }

    void checkAppendHeader()
    {
        if(shouldAppendHeader_)
        {
            message_.header.push_back(std::make_pair<std::string, std::string>(move(tmpField_), move(tmpValue_)));
            shouldAppendHeader_ = false;
        }
    }

    void clear()
    {
        shouldAppendHeader_ = false;
        tmpField_.clear();
        tmpValue_.clear();
        message_ = HttpMessage{};
    }

    http_parser_settings settings_;

    http_parser parser_;

    std::string tmpField_;
    std::string tmpValue_;

    HttpMessage message_;

    bool shouldAppendHeader_;

    std::function<void(HttpMessage &)> messageHandler_;
};

