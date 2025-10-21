#pragma once

#include <boost/signals2.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include "Common.hpp"

class MessageParser {
public:
    // Signal emitted when a valid message is parsed
    boost::signals2::signal<void(const Common::NetMessage&)> messageParsed;
    
    // Signal emitted when parsing fails
    boost::signals2::signal<void(const std::string&)> parseError;
    
    MessageParser();
    ~MessageParser() = default;
    
    // Parse incoming UDP data
    void parseData(const std::vector<uint8_t>& data);
        
private:
    bool validateMessage_(const Common::NetMessage& message, const std::vector<uint8_t>& rawData);
    Common::NetMessage parseMessage_(const std::vector<uint8_t>& data);
    
};