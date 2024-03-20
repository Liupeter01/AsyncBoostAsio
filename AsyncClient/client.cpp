#include<iostream>
#include<boost/asio.hpp>
#include"msg.pb.h"

constexpr std::size_t HEAD_LENGTH = 2;
constexpr std::size_t MAX_LENGTH = 2 * 1024;

int main()
{
		  try
		  {
					boost::asio::io_context ioc;
					boost::asio::ip::tcp::endpoint remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 9876);
					boost::asio::ip::tcp::socket sock(ioc);
					boost::system::error_code err = boost::asio::error::host_not_found;
					sock.connect(remote_ep, err);
					if (err) {
							  std::cout << "connection failed, code is " << err.value()
										<< "error msg is " << err.message() << std::endl;
							  return 0;
					}

					std::thread send_thread([&sock]() {
							  for (;;) {
										std::this_thread::sleep_for(std::chrono::milliseconds(1));
										book bookdata;
										bookdata.set_name("hello world");
										bookdata.set_pages(1000);
										bookdata.set_price(199.99);
										std::string request;
										bookdata.SerializeToString(&request);

										auto request_length = boost::asio::detail::socket_ops::host_to_network_short(request.length());

										char send_data[MAX_LENGTH]{ 0 };
										std::memcpy(send_data, &request_length, HEAD_LENGTH);
										std::memcpy(send_data + HEAD_LENGTH, request.c_str(), request_length);
										boost::asio::write(sock, boost::asio::buffer(send_data, request_length + HEAD_LENGTH));
							  }
				    });

					std::thread recv_thread([&sock]() {
							  for (;;) {
										std::this_thread::sleep_for(std::chrono::milliseconds(1));
										char reply_head[HEAD_LENGTH]{ 0 };
										char reply_msg[MAX_LENGTH]{ 0 };

										std::size_t reply_head_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
										int16_t  msg_length = boost::asio::detail::socket_ops::network_to_host_short(*(int16_t*)reply_head);
										std::size_t replay_msg_length = boost::asio::read(sock, boost::asio::buffer(reply_msg, msg_length));

										book bookdata;
										bookdata.ParseFromArray(reply_msg, msg_length);
										std::cout << "Reply Msg is : ";
										std::cout.write(reply_msg, msg_length);
										std::cout<< "\n";
							  }
					});
					send_thread.join();
					recv_thread.join();
		  }
		  catch (const boost::system::system_error& e)
		  {
					std::cerr << e.what() << std::endl;
		  }
		  return 0;
}