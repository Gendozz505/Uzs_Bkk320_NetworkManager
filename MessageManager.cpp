#include "MessageManager.hpp"
#include <nlohmann/json.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/types.h>
#include <vector>

using json = nlohmann::json;

enum Command : uint8_t {
  IP_REQUEST = 0xF6,
  IP_RESPONSE = 0xF7,
};

MessageManager::MessageManager(boost::asio::io_context &ioContext, const std::string &mainCfgFile) : strand_(boost::asio::make_strand(ioContext)) {
    // Set main configuration file path
    bkk32Info_.setMainCfgPath(mainCfgFile);
}

void MessageManager::processMessage(Common::NetMessage &message, boost::asio::ip::udp::endpoint &remoteEndpoint) {
  boost::asio::post(strand_, [this, message = std::move(message), remoteEndpoint = std::move(remoteEndpoint)]() mutable {
    processMessage_(message, remoteEndpoint);
  });
}

void MessageManager::processMessage_(Common::NetMessage &message, boost::asio::ip::udp::endpoint &remoteEndpoint) {
  try {
    validateMessage_(message);

    std::vector<uint8_t> response;
    processCommand_(message.cmd, response);

    // Signal the message is ready to be sent via UDP
    onUdpReadyToSend(response, remoteEndpoint);

  } catch (const std::exception &e) {
    std::string error = std::string("Message processing failed: ") + e.what();
    spdlog::error("[MessageManager] {}", error);
  }
}

void MessageManager::validateMessage_(const Common::NetMessage &message) {
  if (message.dataLen != message.payload.size()) {
    throw std::runtime_error("Data length mismatch");
  }
}

void MessageManager::processCommand_(uint8_t &cmd, std::vector<uint8_t> &response) {
  switch (cmd) {
  case Command::IP_REQUEST: {
    ipRequestHandler_(response);
    break;
  }
  default: {
    spdlog::warn("[MessageManager] Unknown command: 0x{:02X}", cmd);
    break;
  }
  }
}

void MessageManager::ipRequestHandler_(std::vector<uint8_t> &response) {
  spdlog::debug("[MessageManager] Processing IP request");

  // Validate network configuration
  std::string ipAddress = Common::getIpAddress();
  std::string mask = Common::getMask();
  uint16_t serialNumber = bkk32Info_.getSerialNumber();
  uint8_t status = 0x00;
  
  // Get version string
  char buffer[128] = {0};
  FILE* pipe = popen("dpkg -s bkkservice320-1.1.0.0 2>/dev/null | grep '^Version:' | cut -d' ' -f2", "r");
  std::string version = (pipe && fgets(buffer, sizeof(buffer), pipe)) ? std::string(buffer) : "unknown";
  if (pipe) pclose(pipe);
  if (!version.empty()) version.erase(version.find_last_not_of(" \n\r\t")+1); // trim whitespace

  // Prepare response payload
  json payload;
  payload["Version"] = version;
  payload["Type"] = "Bkk320";
  payload["IP"] = ipAddress;
  payload["MASK"] = mask;
  payload["MODE"] = 1; // Боевой режим

  std::string payloadString = payload.dump();
  spdlog::debug("[MessageManager] Response payload: {}", payloadString);

  const size_t bufferSize = MESSAGE_MIN_SIZE + payloadString.size();
  response.resize(bufferSize);
  uint8_t *pResponse = response.data();
  size_t offset = 0;

  // CMD
  pResponse[offset++] = Command::IP_RESPONSE;

  // Serial Number
  pResponse[offset++] = static_cast<uint8_t>(serialNumber & 0xFF);
  pResponse[offset++] = static_cast<uint8_t>((serialNumber >> 8) & 0xFF);

  // Status
  pResponse[offset++] = status;

  // Data Length
  uint32_t payloadSize = static_cast<uint32_t>(payloadString.size());
  pResponse[offset++] = static_cast<uint8_t>(payloadSize & 0xFF);
  pResponse[offset++] = static_cast<uint8_t>((payloadSize >> 8) & 0xFF);
  pResponse[offset++] = static_cast<uint8_t>((payloadSize >> 16) & 0xFF);
  pResponse[offset++] = static_cast<uint8_t>((payloadSize >> 24) & 0xFF);

  // Payload
  memcpy(pResponse + offset, payloadString.data(), payloadString.size());
  offset += payloadString.size();

  // Calculate CRC16 for all data except CRC field itself
  uint16_t crc = Common::calculateCRC16(pResponse, offset);

  // CRC16
  pResponse[offset++] = static_cast<uint8_t>(crc & 0xFF);       
  pResponse[offset++] = static_cast<uint8_t>((crc >> 8) & 0xFF);
}