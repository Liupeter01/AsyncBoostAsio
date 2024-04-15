#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc, unsigned short port)
          :_ioc(ioc),_acceptor(ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
{
          std::cout << "Server Start On port: " << port << std::endl;
}


void WebSocketServer::startAccept()
{
          auto conn_ptr = std::make_shared<Connection>(_ioc);
          _acceptor.async_accept(conn_ptr->GetSocket(), [conn_ptr,this](boost::system::error_code ec) {
					try{
                              if (!ec) {
                                        conn_ptr->AsyncAccept();
                              }
                              else{
                                        std::cout << "Acceptor failed, err code = " << ec.what() << std::endl;
                              }
                              startAccept();
					}
					catch (const std::exception& e){
                              std::cerr << "StartAccept Error " << e.what() << "\n";
					}
           });
}