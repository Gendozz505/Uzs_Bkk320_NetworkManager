#include "MessageParser.hpp"
#include "Common.hpp"
#include <cstring>
#include <spdlog/spdlog.h>

MessageParser::MessageParser(boost::asio::io_context &ioContext) : strand_(boost::asio::make_strand(ioContext)) {}

void MessageParser::parseData(std::vector<uint8_t> &&data, boost::asio::ip::udp::endpoint &&remoteEndpoint) {
  if (data.size() < MESSAGE_MIN_SIZE) {
    onParseFailed("Message too short");
    return;
  }

  try {
    Common::NetMessage message = parseMessage_(data);

    if (validateMessage_(message, data)) {
      spdlog::trace(
          "[MessageParser] Valid message parsed - CMD: {:#x}, SERIAL NUMBER: {}, STATUS: {:#x}, DATALEN: {}, PAYLOAD: {}",
          message.cmd, message.serialNumber, message.status, message.dataLen,
          Common::u8BytesToHex(message.payload.data(), message.payload.size()));

      onMessageParsed(message, remoteEndpoint);
    } else {
      onParseFailed("Message validation failed");
    }
  } catch (const std::exception &e) {
    onParseFailed(std::string("Parse error: ") + e.what());
  }
}

Common::NetMessage MessageParser::parseMessage_(const std::vector<uint8_t> &data) {
  Common::NetMessage message;
  size_t offset = 0;

  // Parse CMD
  message.cmd = data[offset++];

  // Parse SERIAL NUMBER
  message.serialNumber = static_cast<uint16_t>(data[offset++]) |
                         (static_cast<uint16_t>(data[offset++] << 8));

  // Parse STATUS
  message.status = data[offset++];

  // Parse DATALEN
  message.dataLen = static_cast<uint32_t>(data[offset++]) |
                    (static_cast<uint32_t>(data[offset++]) << 8) |
                    (static_cast<uint32_t>(data[offset++]) << 16) |
                    (static_cast<uint32_t>(data[offset++] << 24));

  // Extract data payload
  if (offset + message.dataLen <= data.size() - sizeof(Common::NetMessage::crc)) {
    // Reverse the payload data
    message.payload.resize(message.dataLen);
    for (int i = (message.dataLen - 1); i >= 0; i--) {
      message.payload[i] = data[offset++];
    }
  } else {
    throw std::runtime_error("Data length exceeds available bytes");
  }

  // Parse CRC16
  message.crc = (static_cast<uint32_t>(data[offset++])) |
                static_cast<uint32_t>(data[offset++] << 8);

  return message;
}

bool MessageParser::validateMessage_(const Common::NetMessage &message,
                                     const std::vector<uint8_t> &rawData) {
  // Check if the message structure is valid
  if (rawData.size() < MESSAGE_MIN_SIZE + message.dataLen) {
    spdlog::warn("[MessageParser] Message size mismatch");
    return false;
  }

  uint16_t calculatedCrc = Common::calculateCRC16(
      rawData.data(), rawData.size() - sizeof(Common::NetMessage::crc));

  if (calculatedCrc != message.crc) {
    spdlog::warn(
        "[MessageParser] CRC16 mismatch - expected: {}, calculated: {}",
        message.crc, calculatedCrc);
    return false;
  }

  return true;
}