#pragma once

#include "Common.hpp"
#include <boost/signals2.hpp>

class MessageManager {
public:
  // Signal to send UDP message
  boost::signals2::signal<void(const std::vector<uint8_t> &)> sendUdpMessage;

  MessageManager();
  ~MessageManager() = default;

  // Process incoming message
  void processMessage(const Common::NetMessage &message);

private:
  void handleParseError(const std::string &error);

  // Message processing logic
  void processCommand_(const Common::NetMessage &message);
  void ipRequestHandler_(const Common::NetMessage &message);
  void validateMessage_(const Common::NetMessage &message);
};
