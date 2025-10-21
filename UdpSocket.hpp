#pragma once

#include "Common.hpp"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <array>
#include <memory>

class UdpSocket {
public:
  // Signal emitted when a udp message is received
  boost::signals2::signal<void(const std::vector<uint8_t>&)> udpMessageReceived;

  UdpSocket(boost::asio::io_context &io,
            const boost::asio::ip::udp::endpoint &endpoint);

  void startReceive();
  void stop();
  
  // Set parser and message manager
  void sendMessage(const std::vector<uint8_t> &buffer);

private:
  void doReceive_();
  void doSend_(std::size_t length);

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remoteEndpoint_;
  std::array<char, 4096> buffer_{};
  bool running_;
  uint64_t sessionId_;
};


