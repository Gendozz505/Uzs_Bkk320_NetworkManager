#pragma once

#include "Common.hpp"
#include "Bkk32Info.hpp"
#include <boost/signals2.hpp>
#include <boost/asio.hpp>

class MessageManager {
public:
  // Signal to send UDP message
  boost::signals2::signal<void(const std::vector<uint8_t> &, const boost::asio::ip::udp::endpoint &)> onUdpReadyToSend;

  MessageManager(boost::asio::io_context &ioContext, const std::string &mainCfgFile);
  ~MessageManager() = default;

  // Process incoming message
  void processMessage(Common::NetMessage &&message, boost::asio::ip::udp::endpoint &&remoteEndpoint);

  boost::asio::strand<boost::asio::io_context::executor_type> getStrand() const { return strand_; }

private:
  // Message processing logic
  void processCommand_(uint8_t &cmd, std::vector<uint8_t> &responseData);
  void ipRequestHandler_(std::vector<uint8_t> &responseData);
  void validateMessage_(const Common::NetMessage &message);

  // Bkk32Info
  Bkk32Info bkk32Info_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
};
