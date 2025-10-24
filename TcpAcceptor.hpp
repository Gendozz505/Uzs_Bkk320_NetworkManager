#pragma once

#include <boost/asio.hpp>

class TcpAcceptor {
public:
    TcpAcceptor(boost::asio::io_context& io, const unsigned short &port);
    ~TcpAcceptor() = default;

    void startAccept();
    void stop();

private:
    void doAccept();

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    bool running_;
    unsigned short port_;
};


