#pragma once

#include <boost/asio.hpp>
#include <array>
#include <memory>
#include <string>
#include <spdlog/spdlog.h>

class TcpSession : public std::enable_shared_from_this<TcpSession> {
public:
    explicit TcpSession(boost::asio::ip::tcp::socket socket);

    void start();
    void send(const std::string& message);

private:
    void doRead();
    void doWrite(std::string message);

private:
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 4096> buffer_{};
    std::shared_ptr<spdlog::logger> logName_;
    uint64_t sessionId_;
};


