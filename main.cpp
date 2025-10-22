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
  std::string mainCfgFile = vm["main-cfg-file"].as<std::string>();

  try {
    boost::asio::io_context io;

    SpdlogManager spdlog;

    net::ip::tcp::endpoint ep{net::ip::tcp::v4(), port};
    TcpAcceptor acceptor(io, ep);

    net::ip::udp::endpoint uep{net::ip::udp::v4(), port};
    UdpSocket udp(io, uep);

    MessageParser parser;
    MessageManager messageManager(mainCfgFile);

    // Setup signal handling for graceful shutdown
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code, int) {
      spdlog::info("Shutdown signal received, stopping services...");
      acceptor.stop();
      udp.stop();
      io.stop();
    });

    // Connect parser signals to message manager
    parser.messageParsed.connect([&messageManager](const Common::NetMessage &message) {
      messageManager.processMessage(message);
    });

    // Connect UDP socket's udpMessageReceived signal to parser's parseData
    // method
    udp.udpMessageReceived.connect([&parser](const std::vector<uint8_t> &message) {
      parser.parseData(message);
    });

    // Connect message manager's sendUdpMessage signal to UDP socket's
    // onSend method
    messageManager.sendUdpMessage.connect([&udp](const std::vector<uint8_t> &buffer) { 
      udp.onSend(buffer); 
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
