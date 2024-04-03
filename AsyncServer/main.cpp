#include"AsyncServer.h"
#include"IOThreadPool.h"
#include<mutex>
#include<condition_variable>

bool _b_stop = false;
std::mutex mutex_quit;
std::condition_variable mutex_cond;

int main() 
{
		  try{
					auto ThreadPool = IOThreadPool::getInstance();
					boost::asio::io_context ioc;
					boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

					/*rigister for ASIO*/
					signals.async_wait([&ioc,&ThreadPool](auto, auto) { 
							  ioc.stop();
							  ThreadPool->stopThreadPool();
							  std::unique_lock<std::mutex> _lckg(mutex_quit);
							  _b_stop = true;
							  mutex_cond.notify_one();
		            });

					/*Main Thread*/
					AsyncServer server(ThreadPool->GetIOContext(), 10086); //sub thread
					{
							  std::unique_lock<std::mutex> _lckg(mutex_quit);
							  mutex_cond.wait(_lckg, []() {return !_b_stop; });
					}
					ioc.run();			//main thread
		  }
		  catch (const std::exception&e){
					std::cerr << "Exception " << e.what();
		  }
          return 0;
}