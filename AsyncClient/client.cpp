#include<iostream>
#include<boost/asio.hpp>
#include<json/json.h>
#include<json/value.h>
#include<json/reader.h>

constexpr std::size_t HEAD_ID_LENGTH = 2;
constexpr std::size_t HEAD_TOTAL_LENGTH = 4;
constexpr std::size_t MAX_LENGTH = 2 * 1024;

int main()
{
		  try
		  {
					boost::asio::io_context ioc;
					boost::asio::ip::tcp::endpoint remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 10086);
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
										std::this_thread::sleep_for(std::chrono::milliseconds(1));	/*test only*/

										char send_data[MAX_LENGTH]{ 0 };
										Json::Value root;
										root["id"] = 1001;
										root["data"] = "hello world";
										std::string request = root.toStyledString();

										std::size_t request_length = request.length();
										int16_t network_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);

										*(int16_t*)send_data = boost::asio::detail::socket_ops::host_to_network_short(1001);																	 //msg_id
										*(int16_t*)(send_data + HEAD_ID_LENGTH) = network_length;		 //msg_length
										std::memcpy(send_data + HEAD_TOTAL_LENGTH, request.c_str(), request_length);
										boost::asio::write(sock, boost::asio::buffer(send_data, request_length + HEAD_TOTAL_LENGTH));
							  }
				    });

					std::thread recv_thread([&sock]() {
							  for (;;) {
										std::this_thread::sleep_for(std::chrono::milliseconds(1));
										char reply_head[HEAD_TOTAL_LENGTH]{ 0 };
										char reply_msg[MAX_LENGTH]{ 0 };

										std::size_t reply_head_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL_LENGTH));
										int16_t msg_id = boost::asio::detail::socket_ops::network_to_host_short(*(int16_t*)reply_head);
										int16_t  msg_length = boost::asio::detail::socket_ops::network_to_host_short(*(int16_t*)(reply_head + HEAD_ID_LENGTH));
										
										std::size_t replay_msg_length = boost::asio::read(sock, boost::asio::buffer(reply_msg, msg_length));

										Json::Value root;
										Json::Reader reader;
										reader.parse(reply_msg, reply_msg + msg_length, root);
										std::cout << "receive msg from server, id = " << root["id"] << ", data = " << root["data"] << std::endl;
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