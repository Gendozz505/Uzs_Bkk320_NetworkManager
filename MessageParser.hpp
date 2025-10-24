#pragma once

#include <boost/signals2.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include "Common.hpp"
#include <boost/asio.hpp>
#include <queue>

class MessageParser {
public:
    // Signal emitted when a valid message is parsed
    boost::signals2::signal<void(Common::NetMessage&, boost::asio::ip::udp::endpoint&)> onMessageReady;
    
    // Signal emitted when parsing fails
    boost::signals2::signal<void(const std::string&)> onParseFailed;
    
    MessageParser(boost::asio::io_context &ioContext);
    ~MessageParser() = default;
    
    // Parse incoming UDP data
    void parseData(std::vector<uint8_t> &data, boost::asio::ip::udp::endpoint &remoteEndpoint);
    
    boost::asio::strand<boost::asio::io_context::executor_type> getStrand() const { return strand_; }
    
private:
    void parseData_(std::vector<uint8_t> &data, boost::asio::ip::udp::endpoint &remoteEndpoint);
    
    bool validateMessage_(const Common::NetMessage& message, const std::vector<uint8_t>& rawData);
    
    Common::NetMessage dataToNetMessage_(const std::vector<uint8_t>& data);

    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
};