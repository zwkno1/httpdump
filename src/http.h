#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

typedef boost::asio::ip::address Address;

typedef  boost::asio::ip::tcp::endpoint Endpoint;

typedef boost::beast::http::request<boost::beast::http::string_body> HttpRequest;

typedef boost::beast::http::response<boost::beast::http::string_body> HttpResponse;

typedef boost::beast::http::request_parser<boost::beast::http::string_body> HttpRequestParser;

typedef boost::beast::http::response_parser<boost::beast::http::string_body> HttpResponseParser;
