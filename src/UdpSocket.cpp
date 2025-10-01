#include "UdpSocket.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

UdpSocket::UdpSocket(boost::asio::io_context &io,
                     const boost::asio::ip::udp::endpoint &endpoint)
    : socket_(io), running_(false) {
  socket_.open(endpoint.protocol());
  socket_.set_option(boost::asio::socket_base::reuse_address(true));
  socket_.bind(endpoint);

  if (!logName_) {
    logName_ = spdlog::stdout_color_mt("UdpSocket");
  }
}

void UdpSocket::startReceive() {
  if (running_)
    return;
  running_ = true;
  doReceive();
}

void UdpSocket::stop() {
  SPDLOG_LOGGER_INFO(logName_, "Socket closing...");
  running_ = false;
  boost::system::error_code ec;
  socket_.close(ec);
}

void UdpSocket::doReceive() {
  if (!running_)
    return;
  socket_.async_receive_from(
      boost::asio::buffer(buffer_), remoteEndpoint_,
      [this](boost::system::error_code ec, std::size_t length) {
        if (!ec && length > 0) {
          std::string data(buffer_.data(), length);
          SPDLOG_LOGGER_DEBUG(logName_, "Received data: {}", data);
          // TODO: Process the received data
        } else if (ec) {
          stop();
        }
        doReceive();
      });
}

void UdpSocket::doSend(std::size_t length) {
  socket_.async_send_to(
      boost::asio::buffer(buffer_.data(), length), remoteEndpoint_,
      [this](boost::system::error_code /*ec*/, std::size_t /*bytes*/) {
        // no-op
      });
}
