#pragma once
#ifndef _HTTP_CONNECTION_H_
#define _HTTP_CONNECTION_H_

#include<boost/asio.hpp>
#include<boost/beast/core.hpp>
#include<boost/beast/http.hpp>
#include<boost/beast/version.hpp>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include<iostream>
#include<ctime>

class HTTPConnection :public std::enable_shared_from_this<HTTPConnection>
{
public:
          HTTPConnection(boost::asio::ip::tcp::socket socket);
          virtual ~HTTPConnection(); 

public:
          void start();

private:
          void read_request();
          void check_timeout();
          void process_request();
          void create_post_response();
          void create_response();
          void write_response();

private:
          boost::asio::ip::tcp::socket _socket;
          boost::beast::flat_buffer _buffer{ 8192 };
          boost::beast::http::request<boost::beast::http::dynamic_body> _request;
          boost::beast::http::response<boost::beast::http::dynamic_body> _response;
          boost::beast::net::steady_timer _deadline{ _socket.get_executor(),std::chrono::seconds(60) };
};


#endif


