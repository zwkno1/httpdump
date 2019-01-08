#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

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

struct HttpMessage
{
    struct Version
    {
        Version(std::uint16_t ma = 1, std::uint16_t mi = 1)
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

    std::string toString() const;
};

const char * httpMethodStr(std::uint8_t code);

const char * httpStatusCodeStr(std::uint16_t code);
