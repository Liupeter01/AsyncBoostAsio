#include"HttpConnection.h"

void http_server(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket) {
          acceptor.async_accept(socket, [&](boost::system::error_code ec) {
                    if (!ec) {
                              std::make_shared<HTTPConnection>(std::move(socket))->start();
                    }

                    http_server(acceptor, socket);
                    });
}

int main() 
{
          try
          {
                    const auto address = boost::beast::net::ip::make_address("127.0.0.1");
                    const unsigned short port = static_cast<unsigned short>(8080);
                    boost::beast::net::io_context ioc{ 1 };
                    boost::asio::ip::tcp::acceptor acceptor{ ioc,{address,port} };
                    boost::asio::ip::tcp::socket socket{ ioc };
                    http_server(acceptor, socket);
                    ioc.run();
          }
          catch (const std::exception& e)
          {
                    std::cerr << e.what() << "\n";
                    return -1;
          }
          return 0;
}