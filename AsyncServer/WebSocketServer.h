#pragma once
#ifndef _WEBSOCKETSERVER_H_
#define _WEBSOCKETSERVER_H_
#include"ConnectionMgr.h"

class WebSocketServer
{
public:
          WebSocketServer(boost::asio::io_context& ioc, unsigned short port);
          virtual ~WebSocketServer() {}

          WebSocketServer(const WebSocketServer&) = delete;
          WebSocketServer& operator=(const WebSocketServer&) = delete;

          void startAccept();

private:
          boost::asio::ip::tcp::acceptor _acceptor;
          boost::asio::io_context& _ioc;
};

#endif // !_WEBSOCKETSERVER_H_