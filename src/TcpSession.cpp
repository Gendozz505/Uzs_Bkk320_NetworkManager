#include "TcpSession.hpp"
#include <utility>
#include <spdlog/sinks/stdout_color_sinks.h>

TcpSession::TcpSession(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {
  if (!logName_) {
    logName_ = spdlog::stdout_color_mt("TcpSession");
  }
}

void TcpSession::start() { doRead(); }

void TcpSession::send(const std::string &message) { doWrite(message); }

void TcpSession::doRead() {
  auto self = shared_from_this();
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::string data(buffer_.data(), length);
          SPDLOG_LOGGER_DEBUG(logName_, "Received data: {}", data);
          // TODO: Process the received data
          doRead();
        } else {
          SPDLOG_LOGGER_INFO(logName_, "{}", ec.message());
          self->socket_.close();
        }
      });
}

void TcpSession::doWrite(std::string message) {
  auto self = shared_from_this();
  boost::asio::async_write(
      socket_, boost::asio::buffer(message),
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (ec) {
          spdlog::error("{}", ec.message());
          self->socket_.close();
        }
      });
}
