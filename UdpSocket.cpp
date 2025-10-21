#include "UdpSocket.hpp"
#include "Common.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>

static std::atomic<uint64_t> nextSessionId_ = 1;

UdpSocket::UdpSocket(boost::asio::io_context &io,
                     const boost::asio::ip::udp::endpoint &endpoint)
    : socket_(io), running_(false), sessionId_(nextSessionId_++) {
  socket_.open(endpoint.protocol());
  socket_.set_option(boost::asio::socket_base::reuse_address(true));
  socket_.bind(endpoint);
}

void UdpSocket::startReceive() {
  if (running_)
    return;
  running_ = true;
  doReceive_();
}

void UdpSocket::stop() {
  running_ = false;
  boost::system::error_code ec;
  socket_.close(ec);
  if (ec) {
    spdlog::warn("[UDP] Error closing socket: {}", ec.message());
  }
}

void UdpSocket::doReceive_() {
  if (!running_)
    return;

  socket_.async_receive_from(
      boost::asio::buffer(buffer_), remoteEndpoint_,
      [this](boost::system::error_code ec, std::size_t length) {
        if (!ec && length > 0) {
          // Convert received data to vector for parser
          std::vector<uint8_t> dataVector(buffer_.begin(),
                                          buffer_.begin() + length);

          spdlog::trace("[UDP] Received {} bytes: {}", dataVector.size(),
          Common::u8BytesToHex(dataVector.data(), dataVector.size()));

          // Emit signal to parser
          udpMessageReceived(dataVector);

        } else if (ec) {
          stop();
        }
        doReceive_();
      });
}

void UdpSocket::doSend_(std::size_t length) {
  socket_.async_send_to(
      boost::asio::buffer(buffer_.data(), length), remoteEndpoint_,
      [this](boost::system::error_code /*ec*/, std::size_t /*bytes*/) {
        // no-op
      });
}

void UdpSocket::sendMessage(const std::vector<uint8_t> &buffer) {

  if (buffer.size() > buffer_.size()) {
    spdlog::error("[UDP] Message too large to send");
    return;
  }
  
  std::copy(buffer.begin(), buffer.end(), buffer_.begin());
  spdlog::trace("[UDP] Sending {} bytes, {}", buffer.size(), Common::u8BytesToHex(buffer.data(), buffer.size()));

  doSend_(buffer.size());
}
