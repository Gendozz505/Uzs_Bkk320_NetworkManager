#include "TcpAcceptor.hpp"
#include "TcpSession.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

TcpAcceptor::TcpAcceptor(boost::asio::io_context& io, const boost::asio::ip::tcp::endpoint& endpoint)
    : acceptor_(io), running_(false) {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(boost::asio::socket_base::max_listen_connections);

    if (!logName_) {
      logName_ = spdlog::stdout_color_mt("TcpAcceptor");
    }
}

void TcpAcceptor::startAccept() {
    if (running_) return;
    running_ = true;
    doAccept();
}

void TcpAcceptor::stop() {
    running_ = false;
    boost::system::error_code ec;
    acceptor_.close(ec);
}

void TcpAcceptor::doAccept() {
    if (!running_) return;
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<TcpSession>(std::move(socket))->start();
            }
            doAccept();
        });
}


