#pragma once

#include "Common.hpp"

#include <boost/asio.hpp>
#include <array>
#include <vector>

class UdpSocket {
public:
  UdpSocket(boost::asio::io_context &io,
            const boost::asio::ip::udp::endpoint &endpoint);

  void startReceive();
  void stop();

private:
  void doReceive();
  void doSend(std::size_t length);

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remoteEndpoint_;
  std::array<char, 4096> buffer_{};
  bool running_;
  uint64_t sessionId_;
};


