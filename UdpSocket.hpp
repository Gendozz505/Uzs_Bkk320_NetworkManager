#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <array>
#include <deque>

class UdpSocket {
public:
  // Signal emitted when a udp message is received
  boost::signals2::signal<void(std::vector<uint8_t> &, boost::asio::ip::udp::endpoint &)> onUdpDataReceived;

  UdpSocket(boost::asio::io_context &io, const unsigned short &port);
  ~UdpSocket() = default;
  void startReceive();
  void stop();
  
  boost::asio::strand<boost::asio::io_context::executor_type> getStrand() const { return strand_; }
  
  // Send UDP message
  void doSend(std::vector<uint8_t> &data, boost::asio::ip::udp::endpoint &remoteEndpoint);
  
  private:
  void doReceive_();
  
  void handleSend_(boost::system::error_code ec, std::size_t bytes);
  void doSend_(std::vector<uint8_t> &data, boost::asio::ip::udp::endpoint &remoteEndpoint);
  
  private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::ip::udp::endpoint remoteEndpoint_;
  std::array<char, 4096> buffer_{};
  bool running_;
  uint64_t sessionId_;
  unsigned short port_;

  std::deque<std::pair<std::vector<uint8_t>, boost::asio::ip::udp::endpoint>> sendQueue_;
};


