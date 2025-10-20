#pragma once

#include <boost/signals2.hpp>
#include <cstdint>
#include <vector>
#include <string>

#define MESSAGE_MIN_SIZE 10

struct NetMessage {
    uint8_t cmd;                  // 1 byte - Command
    uint16_t serialNumber;                  // 2 bytes - Serial Number
    uint8_t status;               // 1 byte - Status
    uint32_t dataLen;             // 4 bytes - Data Length
    std::vector<uint8_t> payload; // N bytes - Data Payload
    uint16_t crc;               // 2 bytes - CRC16 checksum

    NetMessage() : cmd(0), serialNumber(0), status(0), dataLen(0), payload(std::vector<uint8_t>()), crc(0) {}
};

class MessageParser {
public:
    // Signal emitted when a valid message is parsed
    boost::signals2::signal<void(const NetMessage&)> messageParsed;
    
    // Signal emitted when parsing fails
    boost::signals2::signal<void(const std::string&)> parseError;
    
    MessageParser();
    ~MessageParser() = default;
    
    // Parse incoming UDP data
    void parseData(const std::vector<uint8_t>& data);
        
private:
    bool validateMessage_(const NetMessage& message, const std::vector<uint8_t>& rawData);
    NetMessage parseMessage_(const std::vector<uint8_t>& data);
    
};