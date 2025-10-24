#include "UdpSocket.hpp"
#include "Common.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>

static std::atomic<uint64_t> nextSessionId_ = 1;

UdpSocket::UdpSocket(boost::asio::io_context &io,
                     const unsigned short &port)
    : strand_(boost::asio::make_strand(io)), socket_(io), running_(false),
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

void UdpSocket::onSend(std::vector<uint8_t> &data,
                       boost::asio::ip::udp::endpoint &remoteEndpoint) {
  boost::asio::post(strand_,
                    [this, data = std::move(data),
                     remoteEndpoint = std::move(remoteEndpoint)]() mutable {
                      onSend_(data, remoteEndpoint);
                    });
}

void UdpSocket::onSend_(std::vector<uint8_t> &data,
                        boost::asio::ip::udp::endpoint &remoteEndpoint) {
  if (data.size() > buffer_.size()) {
    spdlog::error("[UDP] Message too large to send: {} bytes (max: {})",
                  data.size(), buffer_.size());
    return;
  }

  spdlog::trace("[UDP] Sending {} bytes: {}", data.size(),
                Common::u8BytesToHex(data.data(), data.size()));

  doSend_(data, remoteEndpoint);
}

void UdpSocket::doSend_(std::vector<uint8_t> &data,
                        boost::asio::ip::udp::endpoint &remoteEndpoint) {
  if (!running_)
    return;

  socket_.async_send_to(
      boost::asio::buffer(data), remoteEndpoint,
      [this](boost::system::error_code ec, std::size_t bytes) {
        if (ec) {
          spdlog::error("[UDP] Send error: {}", ec.message());
        } else {
          spdlog::trace("[UDP] Sent {} bytes successfully", bytes);
        }
      });
}