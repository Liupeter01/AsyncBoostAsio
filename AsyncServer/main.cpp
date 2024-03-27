#include"AsyncServer.h"
#include<csignal>

bool _b_quit = false;
std::condition_variable mutex_quit_cond;
std::mutex mutex_quit;

void sig_handler(int sig) {
		  if (sig == SIGINT || sig == SIGTERM) {
					std::unique_lock<std::mutex> _lckg(mutex_quit);
					_b_quit = true;
					mutex_quit_cond.notify_one();					  //wake up
		  }
}

int main() 
{
		  try{
					boost::asio::io_context ioc;
					AsyncServer server(ioc, 10086);
					std::thread network_thread([&ioc]() {
							  ioc.run();			//excute iocp/epoll model
				     });
					signal(SIGINT, sig_handler);  //capture SIGINT signal
					signal(SIGTERM, sig_handler);  //capture SIGINT signal

					std::unique_lock<std::mutex> _lckg(mutex_quit);
					mutex_quit_cond.wait(_lckg, []() { return _b_quit; });

					ioc.stop();								   //shutdown model
					network_thread.join();
		  }
		  catch (const std::exception&e){
					std::cerr << "Exception " << e.what();
		  }
          return 0;
}