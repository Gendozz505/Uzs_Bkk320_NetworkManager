#include "MessageManager.hpp"
#include "Bkk32Info.hpp"
#include "MessageParser.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>

#define CMD_IP_REQUEST 0xF6
#define CMD_IP_RESPONSE 0xF7

MessageManager::MessageManager() {
  spdlog::debug("[MessageManager] Initialized");
}

void MessageManager::processMessage(const NetMessage &message) {
  try {
    validateMessage_(message);
    processCommand_(message);

  } catch (const std::exception &e) {
    std::string error = std::string("Message processing failed: ") + e.what();
    spdlog::error("[MessageManager] {}", error);
  }
}

void MessageManager::handleParseError(const std::string &error) {
  spdlog::error("[MessageManager] Parse error: {}", error);
}

void MessageManager::validateMessage_(const NetMessage &message) {
  // Basic validation checks
  if (message.dataLen != message.payload.size()) {
    throw std::runtime_error("Data length mismatch");
  }

  if (message.dataLen != message.payload.size()) {
    throw std::runtime_error("Data length mismatch");
  }
  // Add more validation rules as needed
  // For example, check command ranges, status values, etc.
}

void MessageManager::processCommand_(const NetMessage &message) {
  switch (message.cmd) {
  case CMD_IP_REQUEST: {
    ipRequestHandler_(message);
    break;
  }
  default: {
    spdlog::warn("[MessageManager] Unknown command: 0x{:02X}", message.cmd);
    break;
  }
  }
}

void MessageManager::ipRequestHandler_(const NetMessage &message) {
  uint8_t status = 0x00;

  json payload;
  payload["Version"] = "0.0.0.0";
  payload["Type"] = "Bkk320";
  payload["IP"] = getIpAddress();
  payload["MASK"] = getMask();
  payload["MODE"] = 0;

  std::string payloadString = payload.dump();
  std::vector<uint8_t> payloadBytes(payloadString.begin(), payloadString.end());

  uint16_t crc = calculateCRC16(payloadBytes);

  NetMessage response;
  response.cmd = CMD_IP_RESPONSE;
  response.serialNumber = getSerialNumber();
  response.status = status;
  response.dataLen = static_cast<uint32_t>(payloadBytes.size());
  response.payload = payloadBytes;
  response.crc = crc;

  // Send response message
  sendUdpMessage(response);
}