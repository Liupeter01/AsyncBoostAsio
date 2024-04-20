#pragma once
#ifndef  _ASYNCSERVER_H_
#define _ASYNCSERVER_H_
#endif // ! _ASYNCSERVER_H_

#include<map>
#include<boost/noncopyable.hpp>
#include"Session.h"

namespace BoostAsioProject
{
    class AsyncServer :public  boost::noncopyable_::noncopyable {
    public:
            AsyncServer(boost::asio::io_context& ioc, unsigned short port);
            void CloseSession(const std::string& _uuid);

    private:
            void StartAccept();
            void HandleAccept(std::shared_ptr<BoostAsioProject::Session>session, boost::system::error_code error);

    private:
            boost::asio::io_context& _ioc;
            boost::asio::ip::tcp::acceptor _acceptor;
            std::map<std::string, std::shared_ptr<BoostAsioProject::Session>> _sessions;
    };
}