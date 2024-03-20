#include "AsyncServer.h"

AsyncServer::AsyncServer(boost::asio::io_context& ioc, unsigned short port)
          :_ioc(ioc), _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), port)){
          std::cout << "Server start success,listen on port:" << port << std::endl;
          this->StartAccept();
}

void AsyncServer::CloseSession(const std::string& _uuid) {
          _sessions.erase(_uuid);
}

void AsyncServer::StartAccept(){
          std::shared_ptr<Session> session_ptr = std::make_shared<Session>(_ioc, this);
          _acceptor.async_accept(session_ptr->Socket(), std::bind(&AsyncServer::HandleAccept, this, session_ptr, std::placeholders::_1));
}

void AsyncServer::HandleAccept(std::shared_ptr<Session>session, boost::system::error_code error) {
          if (!error.value()) {
                    session->Start();
                    _sessions.insert(std::make_pair(session->GetUuid(), session));
          }
          else {
                    this->CloseSession(session->GetUuid());
          }
          StartAccept();
}