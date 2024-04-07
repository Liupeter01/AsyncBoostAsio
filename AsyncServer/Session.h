#pragma once
#ifndef  _SESSION_H_
#define _SESSION_H_

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

/*Activate Coroutine*/
#include<boost/asio/co_spawn.hpp>
#include<boost/asio/detached.hpp>

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
          virtual ~Session();
          boost::asio::ip::tcp::socket& Socket();
          std::string& GetUuid();
          void Start();
          void Close();

          void Send(std::string str, short msg_id);
          void Send(const char* msg, std::size_t max_length, short msg_id);

private:
          void handle_write(std::shared_ptr<Session> _self_shared, boost::system::error_code error);

private:
          boost::asio::io_context& _io_context;
          boost::asio::ip::tcp::socket _socket;

          std::string _uuid_str;
          AsyncServer* _server;

		  std::queue < std::shared_ptr<SendNode>> _send_queue;
		  std::mutex _send_mutex;

		  std::shared_ptr<RecvNode> _recv_msg_node;			//收到的消息结构
		  std::shared_ptr<MsgHead> _recv_head_node;		//收到的头部结构
		  bool _b_head_parse;
          bool _b_stop_session;
};
#endif // ! _SESSION_H_