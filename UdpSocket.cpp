#include "UdpSocket.hpp"
#include "Common.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

static std::atomic<uint64_t> nextSessionId_ = 1;

UdpSocket::UdpSocket(boost::asio::io_context &io,
                     const unsigned short &port)
    : socket_(io), strand_(boost::asio::make_strand(io)), running_(false),
      sessionId_(nextSessionId_++), port_(port) {
  boost::asio::ip::udp::endpoint endpoint{boost::asio::ip::udp::v4(), port};
  socket_.open(endpoint.protocol());
  socket_.set_option(boost::asio::socket_base::reuse_address(true));
  socket_.bind(endpoint);
}

void UdpSocket::stop() {
  running_ = false;
  boost::system::error_code ec;
  socket_.close(ec);
  if (ec) {
    spdlog::warn("[UDP] Error closing socket: {}", ec.message());
  }
}

void UdpSocket::startReceive() {
  if (running_)
    return;
  running_ = true;
  doReceive_();
}

void UdpSocket::doReceive_() {

  boost::asio::post(strand_, [this]() {
    socket_.async_receive_from(
        boost::asio::buffer(buffer_), remoteEndpoint_,
        [this](boost::system::error_code ec, std::size_t length) {
          if (!running_)
            return;

          if (!ec && length > 0) {
            // Convert received data to vector for parser
            std::vector<uint8_t> dataVector(buffer_.begin(),
                                            buffer_.begin() + length);

            spdlog::trace(
                "[UDP] Received {} bytes: {}", dataVector.size(),
                Common::u8BytesToHex(dataVector.data(), dataVector.size()));

            // Emit signal to parser
            onUdpDataReceived(dataVector, remoteEndpoint_);

          } else if (ec) {
            stop();
          }

          // Continue receiving
          doReceive_();
        });
  });
}

void UdpSocket::doSend(std::vector<uint8_t> &data,
                       boost::asio::ip::udp::endpoint &remoteEndpoint) {
  boost::asio::post(strand_,
                    [this, data = std::move(data),
                     remoteEndpoint = std::move(remoteEndpoint)]() mutable {
                      doSend_(data, remoteEndpoint);
                    });
}

void UdpSocket::doSend_(std::vector<uint8_t> &data,
                        boost::asio::ip::udp::endpoint &remoteEndpoint) {
  if (!running_)
    return;

  if (data.size() > buffer_.size()) {
    spdlog::error("[UDP] Message too large to send: {} bytes (max: {})",
                  data.size(), buffer_.size());
    return;
  }

  // If queue is empty, we can send the message immediately
  bool isReadyToSend = sendQueue_.empty();

  sendQueue_.push_back(std::make_pair(std::move(data), remoteEndpoint));

  if (!isReadyToSend) {
    return;
  }

  socket_.async_send_to(
      boost::asio::buffer(sendQueue_.front().first), sendQueue_.front().second,
      [this](boost::system::error_code ec, std::size_t bytes) { handleSend_(ec, bytes); });
}

void UdpSocket::handleSend_(boost::system::error_code ec, std::size_t bytes) {
  if (ec) {
    spdlog::error("[UDP] Send error: {}", ec.message());
  } 
  
  else if (bytes > 0) {
    spdlog::trace("[UDP] Sent {} bytes (out of {}): {}", bytes,sendQueue_.front().first.size(),
      Common::u8BytesToHex(sendQueue_.front().first.data(), bytes));
  }

  sendQueue_.pop_front();

  if (!sendQueue_.empty()) {
    socket_.async_send_to(
      boost::asio::buffer(sendQueue_.front().first), sendQueue_.front().second,
      [this](boost::system::error_code ec, std::size_t bytes) { handleSend_(ec, bytes); });
  }
}