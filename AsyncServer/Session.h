#pragma once
#ifndef  _SESSION_H_
#define _SESSION_H_
#include"const.h"
#include<iostream>
#include<queue>
#include<mutex>
#include<memory>
#include<boost/asio.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>

class AsyncServer;
class Session;

constexpr std::size_t HEAD_LENGTH = 2;
constexpr std::size_t MAX_LENGTH = 2 * 1024;

class MsgNode {
		  friend class Session;
public:
		  //负责数据的读取
		  MsgNode(std::size_t total_length) 
					:_total_length(total_length), _cur_length(0), _msg(new char [total_length + 1] {0}) {
		  }
		  //负责数据的写入
		  MsgNode(const char* msg, std::size_t total_length) 
					:_total_length(total_length + HEAD_LENGTH), _cur_length(0), _msg(new char [total_length + HEAD_LENGTH + 1] {0}) {
					/*修改为网络字节序*/
					int max_len_host = boost::asio::detail::socket_ops::host_to_network_short(total_length);
					std::memcpy(_msg, &total_length, HEAD_LENGTH);
					std::memcpy(_msg + HEAD_LENGTH, msg, max_len_host);
					_msg[_total_length] = '\0';
		  }
		  void clear() {
					std::memset(_msg, 0, _total_length);
					_cur_length = 0;
		  }
		  virtual ~MsgNode() { delete []_msg; }

private:
		  char* _msg;
		  std::size_t _total_length;
		  std::size_t _cur_length;
};

class Session :public std::enable_shared_from_this<Session>
{
public:
          Session(boost::asio::io_context& ioc, AsyncServer* server);
          boost::asio::ip::tcp::socket& Socket();
          std::string& GetUuid();
          void Start();
		  void Send(char* msg, int max_length);

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

		  std::queue < std::shared_ptr<MsgNode>> _send_queue;
		  std::mutex _send_mutex;

		  std::shared_ptr<MsgNode> _recv_msg_node;			//收到的消息结构
		  std::shared_ptr<MsgNode> _recv_head_node;		//收到的头部结构
		  bool _b_head_parse;
};
#endif // ! _SESSION_H_