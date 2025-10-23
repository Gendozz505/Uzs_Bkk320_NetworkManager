#include "MessageManager.hpp"
#include "MessageParser.hpp"
#include "SpdlogManager.hpp"
#include "TcpAcceptor.hpp"
#include "UdpSocket.hpp"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <csignal>
#include <iostream>

namespace po = boost::program_options;
namespace net = boost::asio;

int main(int argc, char **argv) {
  // Define available options
  po::options_description desc("Usage: uzsBkk320NetworkManager [options]");
  desc.add_options()
    ("help,h", "Display help message")
    ("port,p", po::value<unsigned short>()->default_value(30720),"The port to listen on (e.g. 30720)")
    ("log-level,l", po::value<std::string>()->default_value("info"),"The log level (e.g. trace, debug, info, warn, error, critical)")
    ("main-cfg-file,m", po::value<std::string>()->default_value(BKK32_DEFAULT_MAIN_CFG_PATH),"Path to main configuration file");

  // Parse command-line arguments
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      return EXIT_SUCCESS;
    }

    po::notify(vm); // Trigger validation of required options
  } catch (const po::error &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    std::cerr << desc << '\n';
    return EXIT_FAILURE;
  }

  unsigned short port = vm["port"].as<unsigned short>();
  std::string logLevel = vm["log-level"].as<std::string>();
  std::string mainCfgFile = vm["main-cfg-file"].as<std::string>();

  try {
    boost::asio::io_context io;

    SpdlogManager spdlog(logLevel);

    net::ip::tcp::endpoint ep{net::ip::tcp::v4(), port};
    TcpAcceptor acceptor(io, ep);

    net::ip::udp::endpoint uep{net::ip::udp::v4(), port};
    UdpSocket udp(io, uep);

    MessageParser parser(io);
    MessageManager messageManager(io, mainCfgFile);

    // Setup signal handling for graceful shutdown
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code, int) {
      spdlog::info("Shutdown signal received, stopping services...");
      acceptor.stop();
      udp.stop();
      io.stop();
    });
    
    // Connect UDP socket's onUdpReceived signal to parser's parseData method
    udp.onUdpReceived.connect([&parser](const std::vector<uint8_t> &data, const boost::asio::ip::udp::endpoint &remoteEndpoint) {
      boost::asio::post(parser.getStrand(), [&parser, data, remoteEndpoint]() {
        parser.parseData(std::move(const_cast<std::vector<uint8_t>&>(data)), std::move(const_cast<boost::asio::ip::udp::endpoint&>(remoteEndpoint)));
      });
    });

    // Connect parser signals to message manager
    parser.onMessageParsed.connect([&messageManager](const Common::NetMessage &message, const boost::asio::ip::udp::endpoint &remoteEndpoint) {
      boost::asio::post(messageManager.getStrand(), [&messageManager, message, remoteEndpoint]() {
        messageManager.processMessage(std::move(const_cast<Common::NetMessage&>(message)), std::move(const_cast<boost::asio::ip::udp::endpoint&>(remoteEndpoint)));
      });
    });
    
    // Connect message manager's sendUdpMessage signal to UDP socket's
    // onSend method
    messageManager.onUdpReadyToSend.connect([&udp](const std::vector<uint8_t> &data, const boost::asio::ip::udp::endpoint &remoteEndpoint) {
      boost::asio::post(udp.getStrand(), [&udp, data, remoteEndpoint]() {
        udp.onSend(std::move(const_cast<std::vector<uint8_t>&>(data)), std::move(const_cast<boost::asio::ip::udp::endpoint&>(remoteEndpoint)));
      });
    });

    spdlog.init();
    acceptor.startAccept();
    udp.startReceive();

    spdlog::info("Server listening on {}:{}", Common::getIpAddress(), port);

    // Run the IO context - blocks until io.stop() is called
    io.run();

    // Cleanup after io.run() returns
    spdlog::info("Server stopped");
    return 0;
  } catch (const std::exception &ex) {
    spdlog::info("{}", ex.what());
    return 1;
  }
}
