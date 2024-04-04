#pragma once
#ifndef  _SESSION_H_
#define _SESSION_H_

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>
#include <memory>

#include "const.h"
#include "MsgNode.h"

class AsyncServer;

class Session :public std::enable_shared_from_this<Session>
{
public:
          Session(boost::asio::io_context& ioc, AsyncServer* server);
          boost::asio::ip::tcp::socket& Socket();
          std::string& GetUuid();
          void Start();
          void Send(std::string str, short msg_id);
          void Send(const char* msg, std::size_t max_length, short msg_id);

private:
          void handle_read(std::shared_ptr<Session> _self_shared, boost::system::error_code error, std::size_t bytes_transferred);
          void handle_write(std::shared_ptr<Session> _self_shared, boost::system::error_code error);

private:
          enum { max_length = MAX_LENGTH};
          boost::asio::ip::tcp::socket _socket;
          //std::unique_ptr<char> _data;
		  char _data[MAX_LENGTH];

          std::string _uuid_str;
          AsyncServer* _server;

		  std::queue < std::shared_ptr<SendNode>> _send_queue;
		  std::mutex _send_mutex;

		  std::shared_ptr<RecvNode> _recv_msg_node;			//收到的消息结构
		  std::shared_ptr<MsgHead> _recv_head_node;		//收到的头部结构
		  bool _b_head_parse;
};
#endif // ! _SESSION_H_