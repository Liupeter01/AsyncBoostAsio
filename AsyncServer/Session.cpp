#include "Session.h"
#include "AsyncServer.h"
#include <iostream>
#include <sstream>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include "SyncLogic.h"

Session::Session(boost::asio::io_context& ioc, AsyncServer* server)
          :_socket(ioc), _server(server), _b_head_parse(false){
          boost::uuids::uuid uuid = boost::uuids::random_generator()();
          _uuid_str = boost::uuids::to_string(uuid);
          _recv_head_node = std::make_shared<MsgHead>(HEAD_TOTAL_LENGTH);
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
          std::memset(_data, 0, MAX_LENGTH);
          _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&Session::handle_read, this, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
          );
}

void  Session::Send(std::string str, short msg_id)
{
          Send(str.c_str(), str.length(), msg_id);
}

void Session::Send(const char* msg, int max_length, short msg_id) {
          std::lock_guard<std::mutex> _lckg(_send_mutex);
          int send_que_size = _send_queue.size();
          if (_send_queue.size() > MAX_SEND_QUEUE) {
                    std::cerr << "Session:" << _uuid_str << " _send_queue full!\n";
                    return;
          }
          _send_queue.emplace(std::make_shared<SendNode>(msg, max_length, msg_id));
          if (send_que_size > 0) {
                    return;
          }

          auto& msgnode = _send_queue.front();
          boost::asio::async_write(this->_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_length),
                    std::bind(&Session::handle_write, this, shared_from_this(), std::placeholders::_1));
}

void Session::handle_write(std::shared_ptr<Session> _self_shared, boost::system::error_code error) {
          if (error) {
                    std::cerr << "handle_write error " << __LINE__ << error.what() << "\n";           /*error occured*/
                    _server->CloseSession(_self_shared->GetUuid());
          }
          std::lock_guard<std::mutex> _lckg(_send_mutex);
          _send_queue.pop();
          if (!_send_queue.empty()) {   //it's still not empty
                    auto& msg = _send_queue.front();
                    boost::asio::async_write(this->_socket, boost::asio::buffer(msg->_msg, msg->_total_length),
                              std::bind(&Session::handle_write, this, _self_shared, std::placeholders::_1));
          }
}

void Session::handle_read(std::shared_ptr<Session> _self_shared, boost::system::error_code error, std::size_t bytes_transferred){
          if (error) {
                    std::cerr << "handle_read error " << __LINE__ << error.what() << "\n";           /*error occured*/
                    _server->CloseSession(_self_shared->GetUuid());
          }
          int cur_copy_length(0);                            //have already moved;
          while (bytes_transferred > 0) {                       //still got some unprocessed data
                    /*head is not parsed!*/
                    if (!_b_head_parse) {
                              /*size is even less than the HEAD_LENGTH*/
                              if (bytes_transferred + _recv_head_node->_cur_length < HEAD_TOTAL_LENGTH) {
                                        std::memcpy(_recv_head_node->_msg + _recv_head_node->_cur_length, _data + cur_copy_length, bytes_transferred);
                                        _recv_head_node->_cur_length += bytes_transferred;
                                        memset(_data, 0, MAX_LENGTH);
                                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                                  std::bind(&Session::handle_read, this,_self_shared, std::placeholders::_1, std::placeholders::_2));
                                        return;
                              }

                              int remain_space = HEAD_TOTAL_LENGTH - _recv_head_node->_cur_length;        //get remain space
                              std::memcpy(_recv_head_node->_msg + _recv_head_node->_cur_length, _data + cur_copy_length, remain_space);

                              //update data length which is being processed and remain unprocessed length
                              cur_copy_length += remain_space;
                              bytes_transferred -= remain_space;

                              /*handle msg id data*/
                              int16_t msg_id = boost::asio::detail::socket_ops::network_to_host_short(*(int16_t*)_recv_head_node->_msg);

                              /*send invalid msg id*/
                              if (msg_id > MAX_LENGTH) {
                                        std::cerr << "Invalid Msg id!\n";
                                        _server->CloseSession(_uuid_str);
                                        return;
                              }

                              /*handle msg length*/
                              int16_t data_length = boost::asio::detail::socket_ops::network_to_host_short(*(int16_t*)(_recv_head_node->_msg + HEAD_ID_LENGTH));

                              /*send oversize packet, terminate connection*/
                              if (data_length > MAX_LENGTH) {
                                        std::cerr << "Oversized Packet!\n";
                                        _server->CloseSession(_uuid_str);
                                        return;
                              }
                              if (data_length <=0) {
                                        std::cerr << "illegale Packet!\n";
                                        _server->CloseSession(_uuid_str);
                                        return;
                              }
                              _recv_msg_node = std::make_shared<RecvNode>(data_length, msg_id);

                              /*data recv is not completed! we could save particial data*/
                              if (bytes_transferred < data_length) {
                                        std::memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_length, _data + cur_copy_length, bytes_transferred);
                                        _recv_msg_node->_cur_length += bytes_transferred;
                                        memset(_data, 0, MAX_LENGTH);
                                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                                  std::bind(&Session::handle_read, this,_self_shared, std::placeholders::_1, std::placeholders::_2));

                                        /*_head processing procedure finished!*/
                                        _b_head_parse = true;
                                        return;
                              }

                              /*bytes_transferred >= data_length*/
                              std::memcpy(_recv_msg_node->_msg+ _recv_msg_node->_cur_length, _data + cur_copy_length, data_length);
                              _recv_msg_node->_cur_length += data_length;
                              cur_copy_length += data_length;
                              bytes_transferred -= data_length;
                              _recv_msg_node->_msg[_recv_msg_node->_total_length] = '\0';

                              /*push LogicPair to queue, and logic thread will handle it*/
                              SyncLogic::getInstance()->commitPairToQueue(
                                        std::make_shared<LogicPair>(shared_from_this(), _recv_msg_node)
                              );

                              _b_head_parse = false;        //continue to receive header data
                              _recv_head_node->clear();

                              if (bytes_transferred <= 0) {//receive finished, register read event and return
                                        std::memset(_data, 0, MAX_LENGTH);
                                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                                  std::bind(&Session::handle_read, this,_self_shared, std::placeholders::_1, std::placeholders::_2));
                                        return;
                              }
                              continue;
                    }
                    else {
                              /*We have already deal with head, so we can process it's msg part*/
                              int remain_length = _recv_msg_node->_total_length - _recv_msg_node->_cur_length;
                              if (bytes_transferred < remain_length) {
                                        std::memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_length, _data + cur_copy_length, bytes_transferred);
                                        _recv_msg_node->_cur_length += bytes_transferred;
                                        memset(_data, 0, MAX_LENGTH);
                                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                                  std::bind(&Session::handle_read, this,_self_shared, std::placeholders::_1, std::placeholders::_2));
                                        return;
                              }
                              //bytes_transferred >= remain_length
                              std::memcpy(_recv_msg_node->_msg+ _recv_msg_node->_cur_length, _data + cur_copy_length, remain_length);
                              _recv_msg_node->_cur_length += remain_length;
                              bytes_transferred -= remain_length;
                              cur_copy_length += remain_length;
                              _recv_msg_node->_msg[_recv_msg_node->_total_length] = '\0';

                              /*push LogicPair to queue, and logic thread will handle it*/
                              SyncLogic::getInstance()->commitPairToQueue(
                                        std::make_shared<LogicPair>(shared_from_this(), _recv_msg_node)
                              );

                              _b_head_parse = false;        //continue to receive header data
                              _recv_head_node->clear();
                              if (bytes_transferred <= 0) { //receive finished, register read event and return
                                        std::memset(_data, 0, MAX_LENGTH);
                                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                                  std::bind(&Session::handle_read, this,  _self_shared, std::placeholders::_1, std::placeholders::_2));
                                        return;
                              }
                              continue;
                    }
          }
}