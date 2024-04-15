#include "Connection.h"
#include"ConnectionMgr.h"

Connection::Connection(boost::asio::io_context& ioc) :_ioc(ioc)
, _ws_ptr(std::make_unique<boost::beast::websocket::stream<boost::beast::tcp_stream>>(boost::asio::make_strand(ioc))) 
{
		  boost::uuids::uuid uuid = boost::uuids::random_generator()();
		  _uuid = boost::uuids::to_string(uuid);
		
}

std::string Connection::GetUuid() const
{
		  return _uuid;
}
boost::asio::ip::tcp::socket& Connection::GetSocket()
{
		  return boost::beast::get_lowest_layer(*_ws_ptr).socket();
}

void Connection::AsyncAccept()
{
		  auto self = shared_from_this();
		  _ws_ptr->async_accept([self](boost::system::error_code ec) {
					try {
							  if (!ec) {
										ConnectionMgr::getInstance().addConnection(self);
										self->Start();
							  }
							  else 
										std::cerr << "Websocket Accept Failed! err = " << ec.what() << "\n";
					}
					catch (const std::exception& e) {
							  std::cerr << "WebSocket " << e.what() << "\n";
					}
		  });
}

void Connection::Start()
{
		  auto self = shared_from_this();
		  _ws_ptr->async_read(_recv_buffer, [self](boost::system::error_code ec, std::size_t bytes_transferred) {
					try{
							  if (!ec) {
										self->_ws_ptr->text(self->_ws_ptr->got_text());
										std::string recv_data = boost::beast::buffers_to_string(self->_recv_buffer.data());
										self->_recv_buffer.consume(self->_recv_buffer.size());
										std::cout << "WebSocket Recv msg = " << recv_data << std::endl;

										self->AsyncSend(std::move(recv_data));
							  }
							  else {
										std::cerr << "Websocket Async Read Failed! err = " << ec.what() << "\n";
										ConnectionMgr::getInstance().removeConnection(self->GetUuid());
							  }
					}
					catch (const std::exception& e){
							  std::cerr << "WebSocket " << e.what() << "\n";
							  ConnectionMgr::getInstance().removeConnection(self->GetUuid());
					}
		   });
}

void Connection::AsyncSend(std::string msg)
{
		  {
					std::lock_guard<std::mutex> _lckg(this->_send_mtx);
					int queue_length = _send_queue.size();
					_send_queue.push(msg);
					if (queue_length > 0) {
							  return;
					}
		  }

		  auto self = shared_from_this();
		  _ws_ptr->async_write(boost::asio::buffer(msg.c_str(), msg.length()), [self](boost::system::error_code ec, std::size_t bytes_transferred) {
					try
					{
							  if (!ec) {
										std::string send_msg;
										{
												  std::lock_guard<std::mutex> _lckg(self->_send_mtx);
												  self->_send_queue.pop();
												  if (self->_send_queue.empty()) {
															return;
												  }
												 send_msg = self->_send_queue.front();
										}
										self->AsyncSend(std::move(send_msg));
							  }
							  else {
										std::cerr << "Websocket AsyncWrite Failed! err = " << ec.what() << "\n";
										ConnectionMgr::getInstance().removeConnection(self->GetUuid());
							  }
					}
					catch (const std::exception&e){
							  std::cerr << "WebSocket " << e.what() << "\n";
							  ConnectionMgr::getInstance().removeConnection(self->GetUuid());
					}
		  });

}