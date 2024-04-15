#pragma once
#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include<iostream>
#include<queue>
#include<boost/asio.hpp>
#include<boost/beast.hpp>
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>

class ConnectionMgr;

class Connection :public std::enable_shared_from_this<Connection>
{
public:
		  Connection(boost::asio::io_context& ioc);
		  virtual ~Connection() {}

public:
		  std::string GetUuid() const;
		  boost::asio::ip::tcp::socket& GetSocket();
		  void AsyncAccept();
		  void Start();
		  void AsyncSend(std::string msg);

private:
		  std::unique_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> _ws_ptr;
		  std::string _uuid;
		  boost::asio::io_context& _ioc;
		  boost::beast::flat_buffer _recv_buffer;
		  std::queue<std::string> _send_queue;
		  std::mutex _send_mtx;
};

#endif