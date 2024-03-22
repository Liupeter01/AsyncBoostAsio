#include "Session.h"
#include"AsyncServer.h"

Session::Session(boost::asio::io_context& ioc, AsyncServer* server)
          :_socket(ioc), _server(server), _b_head_parse(false){
          boost::uuids::uuid uuid = boost::uuids::random_generator()();
          _uuid_str = boost::uuids::to_string(uuid);
          _recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
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
          _recv_head_node->clear();
          boost::asio::async_read(_socket,boost::asio::buffer(_recv_head_node->_msg,HEAD_LENGTH),
                    std::bind(&Session::handle_head, this, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
          );
}

void Session::Send(const char* msg, int max_length) {
          std::lock_guard<std::mutex> _lckg(_send_mutex);
          int send_que_size = _send_queue.size();
          if (_send_queue.size() > MAX_SEND_QUEUE) {
                    std::cerr << "Session:" << _uuid_str << " _send_queue full!\n";
                    return;
          }
          _send_queue.emplace(std::make_shared<MsgNode>(msg, max_length));
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

void Session::printBuffer(const char* str, int length){
          std::stringstream ss;
          
          for (int i = 0; i < length; ++i) {
                    std::string hexstr;
                    ss << static_cast<int>(str[i]) << std::endl;
          }
}

void Session::handle_head(std::shared_ptr<Session> _self_shared, boost::system::error_code error, std::size_t bytes_transferred){
          if (error) {
                    std::cerr << "handle_head error " << __LINE__ << error.what() << "\n";           /*error occured*/
                    _server->CloseSession(_self_shared->GetUuid());
                    return;
          }
          if (bytes_transferred < HEAD_LENGTH) {
                    std::cerr << "recv head: head length error" << __LINE__ << "\n";
                    _server->CloseSession(_self_shared->GetUuid());
                    return;
          }
          int16_t msg_length = *((int16_t*)_recv_head_node->_msg);
          if (msg_length > MAX_LENGTH) {
                    std::cout<<"recv head: head length illegal!"<<__LINE__ << "\n";
                    _server->CloseSession(_self_shared->GetUuid());
                    return;
          }

          _recv_msg_node = std::make_shared<MsgNode>(msg_length);
          boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_msg, _recv_msg_node->_total_length),
                    std::bind(&Session::handle_msg, this, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
          );
}

void Session::handle_msg(std::shared_ptr<Session> _self_shared, boost::system::error_code error, std::size_t bytes_transferred){
          if (error) {
                    std::cerr << "handle_read error " << __LINE__ << error.what() << "\n";           /*error occured*/
                    _server->CloseSession(_self_shared->GetUuid());
          }
          _recv_msg_node->_msg[_recv_msg_node->_total_length] = '\0';
          this->Send(_recv_msg_node->_msg, _recv_msg_node->_total_length);

          _recv_head_node->clear();               //clear head

          boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_msg, _recv_msg_node->_total_length),
                    std::bind(&Session::handle_msg, this, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
          );
}