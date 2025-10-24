#include "GlobalControl.hpp"
#include "DataParser.hpp"
#include "MessageManager.hpp"
#include "SpdlogManager.hpp"
#include "TcpAcceptor.hpp"
#include "UdpSocket.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <exception>
#include <vector>

namespace net = boost::asio;

GlobalControl::GlobalControl() {}
GlobalControl::~GlobalControl() {}

int GlobalControl::start(boost::program_options::variables_map &vm) {
  try {
    unsigned short port = vm["port"].as<unsigned short>();
    std::string logLevel = vm["log-level"].as<std::string>();
    std::string mainCfgFile = vm["main-cfg-file"].as<std::string>();

    tcpAcceptor_ = std::make_unique<TcpAcceptor>(io_, port);
    udpSocket_ = std::make_unique<UdpSocket>(io_, port);
    dataParser_ = std::make_unique<DataParser>(io_);
    messageManager_ = std::make_unique<MessageManager>(io_, mainCfgFile);

    // Bind signals to the appropriate handlers
    bindSignals_();

    // Setup signal handling for graceful shutdown
    net::signal_set signals(io_, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code, int) {
      spdlog::info("Shutdown signal received, stopping services...");
      tcpAcceptor_->stop();
      udpSocket_->stop();
      io_.stop();
    });

    spdlog_.init(logLevel);
    tcpAcceptor_->startAccept();
    udpSocket_->startReceive();

    spdlog::info("Server listening on {}:{}", Common::getIpAddress(), port);

    // Run the IO context - blocks until io_.stop() is called
    io_.run();

    // Cleanup after io_.run() returns
    spdlog::info("Server stopped");
    return 0;
  } catch (const std::exception &ex) {
    spdlog::info("{}", ex.what());
    return 1;
  }
}

void GlobalControl::bindSignals_() {
  // Connect onUdpDataReceived to dataParser_->parseData()
  udpSocket_->onUdpDataReceived.connect(
      [this](std::vector<uint8_t> &data,
             boost::asio::ip::udp::endpoint &remoteEndpoint) {
        dataParser_->parseData(data, remoteEndpoint);
      });

  // Connect onMessageReady to messageManager_->processMessage()
  dataParser_->onMessageReady.connect(
      [this](Common::NetMessage &message,
             boost::asio::ip::udp::endpoint &remoteEndpoint) {
        messageManager_->processMessage(message, remoteEndpoint);
      });

  // Connect onUdpReadyToSend to udpSocket_->onSend()
  messageManager_->onUdpReadyToSend.connect(
      [this](std::vector<uint8_t> &response,
             boost::asio::ip::udp::endpoint &remoteEndpoint) {
        udpSocket_->onSend(response, remoteEndpoint);
      });
}