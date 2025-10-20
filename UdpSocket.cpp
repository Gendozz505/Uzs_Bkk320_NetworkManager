#include "UdpSocket.hpp"
#include "MessageManager.hpp"
#include "MessageParser.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
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
                        u8BytesToHex(dataVector.data(), dataVector.size()));

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

void UdpSocket::sendMessage(const NetMessage &message) {
  // Serialize NetMessage to buffer
  size_t offset = 0;

  // CMD
  buffer_[offset++] = message.cmd;

  // Serial Number
  buffer_[offset++] = (message.serialNumber >> 8) & 0xFF;
  buffer_[offset++] = message.serialNumber & 0xFF;

  // Status
  buffer_[offset++] = message.status;

  // Data Length
  buffer_[offset++] = (message.dataLen >> 24) & 0xFF;
  buffer_[offset++] = (message.dataLen >> 16) & 0xFF;
  buffer_[offset++] = (message.dataLen >> 8) & 0xFF;
  buffer_[offset++] = message.dataLen & 0xFF;

  // Payload
  if (message.payload.size() <= buffer_.size() - offset - 2) {
    for (int i = (message.payload.size() - 1); i >= 0; i--) {
      buffer_[offset++] = message.payload[i];
    }
  } else {
    spdlog::error("[UDP] Message too large to send");
    return;
  }

  // CRC16
  buffer_[offset++] = (message.crc >> 8) & 0xFF;
  buffer_[offset++] = message.crc & 0xFF;

  spdlog::trace("[UDP] Sending {} bytes", offset);
  doSend_(offset);
}
