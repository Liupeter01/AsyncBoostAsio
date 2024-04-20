#include "AsyncServer.h"
#include "IOServicePool.h"

BoostAsioProject::AsyncServer::AsyncServer(boost::asio::io_context& ioc, unsigned short port)
          :_ioc(ioc), _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), port)){
          std::cout << "Server start success,listen on port:" << port << std::endl;
          this->StartAccept();
}

void BoostAsioProject::AsyncServer::CloseSession(const std::string& _uuid) {
          _sessions.erase(_uuid);
}

void BoostAsioProject::AsyncServer::StartAccept(){
          /*Get io_context from IOService, IOServicePool using RR to deliver io_context instance*/
          boost::asio::io_context& ioc_pool = BoostAsioProject::IOServicePool::getInstance()->getIOService();
          std::shared_ptr<BoostAsioProject::Session> session_ptr = std::make_shared<BoostAsioProject::Session>(ioc_pool, this);
          _acceptor.async_accept(session_ptr->Socket(), std::bind(&AsyncServer::HandleAccept, this, session_ptr, std::placeholders::_1));
}

void BoostAsioProject::AsyncServer::HandleAccept(std::shared_ptr<BoostAsioProject::Session> session, boost::system::error_code error) {
          if (!error.value()) {
                    session->Start();
                    _sessions.insert(std::make_pair(session->GetUuid(), session));
          }
          else {
                    this->CloseSession(session->GetUuid());
          }
          StartAccept();
}