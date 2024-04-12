#include"AsyncServer.h"
#include"IOServicePool.h"

int main() 
{
		  try{
					auto ServicePool = IOServicePool::getInstance();

					/*deal with user's interrupt(SIGINT, SIGTERM)*/
					boost::asio::io_context signal_wait;
					boost::asio::signal_set signals(signal_wait, SIGINT, SIGTERM);
					signals.async_wait([&signal_wait,&ServicePool](auto, auto) { 
							  signal_wait.stop();
							  ServicePool->stopServicePool();		  //shutdown all io_context
				    });

					AsyncServer server(signal_wait, 10086);
					signal_wait.run();			//excute iocp/epoll model
		  }
		  catch (const std::exception&e){
					std::cerr << "Exception " << e.what();
		  }
          return 0;
}