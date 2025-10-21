#include "MessageManager.hpp"
#include "Bkk32Info.hpp"
#include <cstdint>
#include <cstring>
#include <spdlog/spdlog.h>
#include <sys/types.h>
#include <vector>

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
  spdlog::debug("[MessageManager] Processing IP request");

  // Validate network configuration
  std::string ipAddress = getIpAddress();
  std::string mask = getMask();
  uint16_t serialNumber = getSerialNumber();

  // Prepare response message
  uint8_t status = 0x00;
  json payload;
  payload["Version"] = "0.0.0.0";
  payload["Type"] = "Bkk320";
  payload["IP"] = ipAddress;
  payload["MASK"] = mask;
  payload["MODE"] = 1;

  std::string payloadString = payload.dump();
  spdlog::debug("[MessageManager] Response payload: {}", payloadString);

  const size_t bufferSize = MESSAGE_MIN_SIZE + payloadString.size();
  std::vector<uint8_t> buffer(bufferSize);
  uint8_t *pBuffer = buffer.data();
  size_t offset = 0;

  // CMD
  pBuffer[offset++] = CMD_IP_RESPONSE;

  // Serial Number (2 bytes, little endian)
  pBuffer[offset++] = static_cast<uint8_t>(serialNumber & 0xFF);
  pBuffer[offset++] = static_cast<uint8_t>((serialNumber >> 8) & 0xFF); // MSB second

  // Status
  pBuffer[offset++] = status;

  // Data Length
  uint32_t payloadSize = static_cast<uint32_t>(payloadString.size());
  pBuffer[offset++] = static_cast<uint8_t>(payloadSize & 0xFF);
  pBuffer[offset++] = static_cast<uint8_t>((payloadSize >> 8) & 0xFF);
  pBuffer[offset++] = static_cast<uint8_t>((payloadSize >> 16) & 0xFF);
  pBuffer[offset++] = static_cast<uint8_t>((payloadSize >> 24) & 0xFF);

  // Payload
  memcpy(pBuffer + offset, payloadString.data(), payloadString.size());
  offset += payloadString.size();

  // Calculate CRC16 for all data except CRC field itself
  uint16_t crc = calculateCRC16(pBuffer, offset);

  // CRC16
  pBuffer[offset++] = static_cast<uint8_t>(crc & 0xFF);       
  pBuffer[offset++] = static_cast<uint8_t>((crc >> 8) & 0xFF);

  // Signal the message is ready to be sent via UDP
  sendUdpMessage(buffer);
}