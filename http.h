#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

#include "detail/http_parser.h"
#include <string>

#include <boost/noncopyable.hpp>

//enum HttpMethod : std::uint8_t
//{
//    HTTP_DELETE = 0,
//    HTTP_GET,
//    HTTP_HEAD,
//    HTTP_POST,
//    HTTP_PUT,
//    /* pathological */
//    HTTP_CONNECT,
//    HTTP_OPTIONS,
//    HTTP_TRACE,
//    /* webdav */
//    HTTP_COPY,
//    HTTP_LOCK,
//    HTTP_MKCOL,
//    HTTP_MOVE,
//    HTTP_PROPFIND,
//    HTTP_PROPPATCH,
//    HTTP_SEARCH,
//    HTTP_UNLOCK,
//    /* subversion */
//    HTTP_REPORT,
//    HTTP_MKACTIVITY,
//    HTTP_CHECKOUT,
//    HTTP_MERGE,
//    /* upnp */
//    HTTP_MSEARCH,
//    HTTP_NOTIFY,
//    HTTP_SUBSCRIBE,
//    HTTP_UNSUBSCRIBE,
//    /* RFC-5789 */
//    HTTP_PATCH,
//    HTTP_PURGE,
//};

inline const char * httpStatusCodeStr(std::uint16_t code)
{
    switch(code)
    {
    case 100:   return "Continue";
    case 101:   return "Switching Protocols";
    case 200:   return "OK";
    case 201:   return "Created";
    case 202:   return "Accepted";
    case 203:   return "Non-Authoritative Information";
    case 204:   return "No Content";
    case 205:   return "Reset Content";
    case 206:   return "Partial Content";
    case 300:   return "Multiple Choices";
    case 301:   return "Moved Permanently";
    case 302:   return "Found";
    case 303:   return "See Other";
    case 304:   return "Not Modified";
    case 305:   return "Use Proxy";
    case 306:   return "Unused";
    case 307:   return "Temporary Redirect";
    case 400:   return "Bad Request";
    case 401:   return "Unauthorized";
    case 402:   return "Payment Required";
    case 403:   return "Forbidden";
    case 404:   return "Not Found";
    case 405:   return "Method Not Allowed";
    case 406:   return "Not Acceptable";
    case 407:   return "Proxy Authentication Required";
    case 408:   return "Request Time-out";
    case 409:   return "Conflict";
    case 410:   return "Gone";
    case 411:   return "Length Required";
    case 412:   return "Precondition Failed";
    case 413:   return "Request Entity Too Large";
    case 414:   return "Request-URI Too Large";
    case 415:   return "Unsupported Media Type";
    case 416:   return "Requested range not satisfiable";
    case 417:   return "Expectation Failed";
    case 500:   return "Internal Server Error";
    case 501:   return "Not Implemented";
    case 502:   return "Bad Gateway";
    case 503:   return "Service Unavailable";
    case 504:   return "Gateway Time-out";
    case 505:   return "HTTP Version not supported";
    default:    return "Unkonown Status";
    }
}

struct HttpMessage
{
    struct Version
    {
        Version(std::uint8_t ma = 1, std::uint8_t mi = 1)
        {
            major = ma;
            minor = mi;
        }
        std::uint16_t major;
        std::uint16_t minor;
    };

    Version version;
    bool isRequest;
    std::uint8_t method;			// request only
    std::string url;				// request only
    std::uint16_t statusCode;		// response only
    std::vector<std::pair<std::string, std::string> > header;
    std::string body;

    std::string toString() const
    {
        std::stringstream ss;
        if(isRequest)
        {
            ss << http_method_str((http_method)method) << " " << url
               << " HTTP/" << version.major << "." << version.minor << "\r\n";
            for(auto & h : header)
            {
                ss << h.first << ": " << h.second << "\r\n";
            }
            ss << "\r\n" << body;
        }
        else
        {
            ss << "HTTP/" << version.major << "." << version.minor
               << " " << statusCode << " " << httpStatusCodeStr(statusCode) << "\r\n";
            for(auto & h : header)
            {
                ss << h.first << ": " << h.second << "\r\n";
            }
            ss << "\r\n" << body;
        }
        return ss.str();
    }
};

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


