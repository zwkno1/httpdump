#pragma once

#include <memory>

#include "tuple4.h"
#include "http.h"

class HttpHandler
{
public:
    //virtual void handleHttpEstablish(const Tuple4 & tuple4) = 0;

    //virtual void handleHttpClose(const Tuple4 & tuple4) = 0;

    virtual bool handleHttpRequest(const Tuple4 & tuple4, HttpRequest & request) = 0;

    virtual bool handleHttpResponse(const Tuple4 & tuple4, HttpResponse & response) = 0;

    virtual ~HttpHandler();
};

HttpHandler::~HttpHandler()
{
}

typedef std::shared_ptr<HttpHandler> HttpHandlerPtr;
