#include "MessageManager.hpp"
#include "MessageParser.hpp"
#include "NetworkManager.hpp"
#include "SpdlogManager.hpp"
#include "TcpAcceptor.hpp"
#include "UdpSocket.hpp"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <csignal>
#include <iostream>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace po = boost::program_options;
namespace net = boost::asio;

namespace {
volatile std::sig_atomic_t g_shouldStop = 0;
void signalHandler(int) { g_shouldStop = 1; }
} // namespace

int main(int argc, char **argv) {
  // Define available options
  po::options_description desc("Usage: uzsBkk320NetworkManager [options]");
  desc.add_options()("help,h", "Display help message")(
      "port,p", po::value<unsigned short>()->default_value(30720),
      "The port to listen on (e.g. 30720)");

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

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  try {
    SpdlogManager spdlog;

    NetworkManager net(std::max(1u, std::thread::hardware_concurrency()));
    auto &io = net.getIoContext();

    net::ip::tcp::endpoint ep{net::ip::tcp::v4(), port};
    TcpAcceptor acceptor(io, ep);

    net::ip::udp::endpoint uep{net::ip::udp::v4(), port};

    UdpSocket udp(io, uep);
    MessageParser parser;
    MessageManager messageManager;

    // Connect parser signals to message manager
    parser.messageParsed.connect([&messageManager](const NetMessage &message) {
      messageManager.processMessage(message);
    });

    // Connect UDP socket's udpMessageReceived signal to parser's parseData
    // method
    udp.udpMessageReceived.connect(
        [&parser](const std::vector<uint8_t> &message) {
          parser.parseData(message);
        });

    // Connect message manager's sendUdpMessage signal to UDP socket's
    // sendMessage method
    messageManager.sendUdpMessage.connect(
        [&udp](const NetMessage &message) { udp.sendMessage(message); });

    spdlog.init();
    net.start();
    acceptor.startAccept();
    udp.startReceive();

    spdlog::info("Server listening on {}:{}", getIpAddress(), port);

    while (!g_shouldStop) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    acceptor.stop();
    udp.stop();
    net.stop();
    spdlog::info("Stopped", port);
    return 0;
  } catch (const std::exception &ex) {
    spdlog::info("{}", ex.what());
    return 1;
  }
}
