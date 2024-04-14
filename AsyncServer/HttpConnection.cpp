#include "HttpConnection.h"

HTTPConnection::HTTPConnection(boost::asio::ip::tcp::socket socket)
          :_socket(std::move(socket)) 
{
}

HTTPConnection::~HTTPConnection()
{
}

void HTTPConnection::start() {
          read_request();
          check_timeout();
}

void  HTTPConnection::read_request()
{
          auto self = shared_from_this();
          boost::beast::http::async_read(_socket, _buffer, _request, [&self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                    boost::ignore_unused(bytes_transferred);
                    if (!ec) {
                              self->process_request();
                    }
                    });
}
void HTTPConnection::check_timeout()
{
          auto self = shared_from_this();
          _deadline.async_wait([this, &self](boost::system::error_code ec) {
                    if (!ec) {
                              self->_socket.close();
                    }
                    });
}

void HTTPConnection::process_request()
{
          _response.version(_request.version());
          _response.keep_alive(false);
          switch (_request.method())
          {
          case boost::beast::http::verb::get:
                    _response.result(boost::beast::http::status::ok);
                    _response.set(boost::beast::http::field::server, "Beast");
                    create_response();
                    break;

          case boost::beast::http::verb::post:
                    _response.result(boost::beast::http::status::ok);
                    _response.set(boost::beast::http::field::server, "Beast");
                    create_post_response();
                    break;

          default:

                    _response.result(boost::beast::http::status::bad_request);
                    _response.set(boost::beast::http::field::content_type, "text/plain");
                    boost::beast::ostream(_response.body()) << " Invalid request-method '"
                              << std::string(_request.method_string()) << " ''";
                    break;
          }
}

void HTTPConnection::create_post_response()
{
          if (_request.target() == "/email") {
                    auto& body = _request.body();
                    auto body_str = boost::beast::buffers_to_string(body.data());

                    _response.set(boost::beast::http::field::content_type, "text/json");
                    Json::Value recv_root;
                    Json::Value send_root;
                    Json::Reader reader;

                    bool status = reader.parse(body_str, recv_root);
                    if (!status) {
                              std::cerr << "Failed to parse Json\n";
                              recv_root["error"] = 1001;
                              boost::beast::ostream(_response.body()) << recv_root.toStyledString();
                              return;
                    }
                    auto get_email = recv_root["email"].asString();
                    send_root["error"] = 0;
                    send_root["email"] = recv_root["email"];
                    send_root["msg"] = "Success!";
                    boost::beast::ostream(_response.body()) << send_root.toStyledString();
          }
          else {
                    _response.result(boost::beast::http::status::not_found);
                    _response.set(boost::beast::http::field::content_type, "text/plain");
                    boost::beast::ostream(_response.body()) << "File Not Found\r\n";
          }
}

void HTTPConnection::create_response()
{
          if (_request.target() == "/count") {
                    _response.set(boost::beast::http::field::content_type, "text/html");
          }
          else if (_request.target() == "/timer") {
                    _response.set(boost::beast::http::field::content_type, "text/html");
          }
          else {
                    _response.result(boost::beast::http::status::not_found);
                    _response.set(boost::beast::http::field::content_type, "text/plain");
                    boost::beast::ostream(_response.body()) << "File Not Found\r\n";
          }
}

void HTTPConnection::write_response()
{
          auto self = shared_from_this();
          _response.content_length(_response.body().size());
          boost::beast::http::async_write(_socket, _response, [this, &self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                    self->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
                    self->_deadline.cancel();                         //shutdown timer
                    });
}
