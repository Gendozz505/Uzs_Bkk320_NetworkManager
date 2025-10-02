#pragma once

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <memory>

class TcpAcceptor {
public:
    TcpAcceptor(boost::asio::io_context& io, const boost::asio::ip::tcp::endpoint& endpoint);

    void startAccept();
    void stop();

    std::string getIpAddress();

private:
    void doAccept();

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    bool running_;
    std::shared_ptr<spdlog::logger> logName_;
};


