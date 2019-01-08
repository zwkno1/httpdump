#pragma once

#include <memory>

#include "tuple4.h"
#include "http.h"

class HttpHandler
{
public:
    /*
     * @direction@ true: client->server, false: server->client
     */
    virtual bool handleHttpMessage(const Tuple4 & tuple4, bool direction, HttpMessage & message) = 0;

    virtual ~HttpHandler() = 0;
};

typedef std::shared_ptr<HttpHandler> HttpHandlerPtr;

