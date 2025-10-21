#include "TcpSession.hpp"
#include <utility>

static std::atomic<uint64_t> nextSessionId_ = 1;

TcpSession::TcpSession(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket)), sessionId_(nextSessionId_++) {
}

void TcpSession::start() {
  spdlog::info("TCP[{}] Session started", sessionId_);
  doRead();
}

void TcpSession::send(const std::string &message) { doWrite(message); }

void TcpSession::doRead() {
  auto self = shared_from_this();
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::string data(buffer_.data(), length);
          spdlog::trace("TCP[{}] RX: {}", sessionId_, data);
          // TODO: Process the received data
          doRead();
        } else {
          spdlog::info("TCP[{}] {}", sessionId_, ec.message());
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
          spdlog::info("TCP[{}] {}", sessionId_, ec.message());
          self->socket_.close();
        }
      });
}
