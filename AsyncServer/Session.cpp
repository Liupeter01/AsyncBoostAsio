#include "Session.h"
#include "AsyncServer.h"
#include <iostream>
#include <sstream>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include "SyncLogic.h"

Session::Session(boost::asio::io_context& ioc, AsyncServer* server)
          :_io_context(ioc), _socket(ioc), _server(server), _b_head_parse(false),_b_stop_session(false) {
          boost::uuids::uuid uuid = boost::uuids::random_generator()();
          _uuid_str = boost::uuids::to_string(uuid);
          _recv_head_node = std::make_shared<MsgHead>(HEAD_TOTAL_LENGTH);
}

Session::~Session()
{
          try {
                    this->Close();
          }
          catch (const std::exception&e){
                    std::cerr << e.what() << std::endl;
          }
}

std::string & Session::GetUuid()
{
          return _uuid_str;
}

boost::asio::ip::tcp::socket& Session::Socket() 
{
          return _socket;
}

void Session::Start() {
          std::shared_ptr<Session> cession_ptr = shared_from_this();

          boost::asio::co_spawn(_io_context,
                    [this, cession_ptr]()->boost::asio::awaitable<void> {
                              try {
                                        for (; !_b_stop_session;) {
                                                  _recv_head_node->clear();
                                                  std::size_t read_length = co_await  boost::asio::async_read(
                                                            _socket, boost::asio::buffer(_recv_head_node->_msg, HEAD_TOTAL_LENGTH), boost::asio::use_awaitable
                                                  );

                                                  if (!read_length) {
                                                            std::cerr << "endpoint closed\n";
                                                            this->Close();                                              //close socket
                                                            _server->CloseSession(_uuid_str);            //close session
                                                            co_return;                                                  //coroutine return
                                                  }

                                                  short msg_id= boost::asio::detail::socket_ops::network_to_host_short(*(short*)_recv_head_node->_msg);
                                                  short msg_length = boost::asio::detail::socket_ops::network_to_host_short(*(short*)(_recv_head_node->_msg + HEAD_ID_LENGTH));
                                                  
                                                  if (msg_length > MAX_LENGTH) {
                                                            std::cout << "recv head: head length illegal!" << __LINE__ << "\n";
                                                            _server->CloseSession(this->GetUuid());
                                                            co_return;
                                                  }

                                                  _recv_msg_node = std::make_shared<RecvNode>(msg_length, msg_id);
                                                  read_length = co_await  boost::asio::async_read(
                                                            _socket, boost::asio::buffer(_recv_msg_node->_msg, msg_length), boost::asio::use_awaitable
                                                  );

                                                  if (!read_length) {
                                                            std::cerr << "endpoint closed\n";
                                                            this->Close();                                              //close socket
                                                            _server->CloseSession(_uuid_str);            //close session
                                                            co_return;                                                  //coroutine return
                                                  }

                                                  _recv_msg_node->_msg[_recv_msg_node->_total_length] = '\0';
                                                  std::cout << "RECEIVED: " << _recv_msg_node->_msg << std::endl;

                                                  SyncLogic::getInstance()->commitPairToQueue(std::make_shared<LogicPair>(cession_ptr, _recv_msg_node));
                                        }
                              }
                              catch (const std::exception&e) {
                                        std::cerr << e.what() << "\n";
                                        this->Close();                                              //close socket
                                        _server->CloseSession(_uuid_str);            //close session
                              }
                    },
                    boost::asio::detached
          );
}

void  Session::Close()
{
          this->_socket.close();
}

void  Session::Send(std::string str, short msg_id)
{
          Send(str.c_str(), str.length(), msg_id);
}

void Session::Send(const char* msg, std::size_t max_length, short msg_id) {
          std::unique_lock<std::mutex> _lckg(_send_mutex);
          std::size_t send_que_size = _send_queue.size();
          if (_send_queue.size() > MAX_SEND_QUEUE) {
                    std::cerr << "Session:" << _uuid_str << " _send_queue full!\n";
                    return;
          }
          _send_queue.emplace(std::make_shared<SendNode>(msg, max_length, msg_id));
          if (send_que_size > 0) {
                    return;
          }

          auto& msgnode = _send_queue.front();
          _lckg.unlock();

          boost::asio::async_write(this->_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_length),
                    std::bind(&Session::handle_write, this, shared_from_this(), std::placeholders::_1));
}

void Session::handle_write(std::shared_ptr<Session> _self_shared, boost::system::error_code error) {
          if (error) {
                    std::cerr << "handle_write error " << __LINE__ << error.what() << "\n";           /*error occured*/
                    this->Close();                                                                                                          //close socket
                    _server->CloseSession(_self_shared->GetUuid());
                    return;
          }
          try{
                    std::unique_lock<std::mutex> _lckg(_send_mutex);
                    _send_queue.pop();
                    if (!_send_queue.empty()) {   //it's still not empty
                              auto& msg = _send_queue.front();
                              _lckg.unlock();

                              boost::asio::async_write(this->_socket, boost::asio::buffer(msg->_msg, msg->_total_length),
                                        std::bind(&Session::handle_write, this, _self_shared, std::placeholders::_1));
                    }
          }
          catch (const std::exception& e) {
                    std::cerr << e.what() << "\n";
                    this->Close();                                              //close socket
                    _server->CloseSession(_uuid_str);            //close session
          }
}