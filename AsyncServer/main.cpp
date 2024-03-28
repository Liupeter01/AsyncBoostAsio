#include"AsyncServer.h"
#include<csignal>

int main() 
{
		  try{
					boost::asio::io_context ioc;
					boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
					signals.async_wait([&ioc](auto, auto) { ioc.stop();});

					AsyncServer server(ioc, 10086);
					ioc.run();			//excute iocp/epoll model
		  }
		  catch (const std::exception&e){
					std::cerr << "Exception " << e.what();
		  }
          return 0;
}