#include"AsyncServer.h"

int main() 
{
		  try{
					boost::asio::io_context ioc;
					AsyncServer server(ioc, 9876);
					ioc.run();			//excute iocp/epoll model
		  }
		  catch (const std::exception&e){
					std::cerr << "Exception " << e.what();
		  }
          return 0;
}