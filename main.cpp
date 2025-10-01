#include "NetworkManager.hpp"
#include "TcpAcceptor.hpp"
#include "UdpSocket.hpp"
#include "SpdlogManager.hpp"

#include <boost/asio.hpp>
#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace {
volatile std::sig_atomic_t g_shouldStop = 0;
void signalHandler(int) { g_shouldStop = 1; }
}

static std::shared_ptr<spdlog::logger> logName_ = spdlog::stdout_color_mt("Main");

int main(int argc, char **argv) {
  unsigned short port = 5555;
  if (argc > 1) {
    port = static_cast<unsigned short>(std::stoi(argv[1]));
  }

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  try {
    SpdlogManager spdlogManager(SPDLOG_LEVEL);

    NetworkManager net(std::max(1u, std::thread::hardware_concurrency()));
    auto &io = net.getIoContext();

    boost::asio::ip::tcp::endpoint ep{boost::asio::ip::tcp::v4(), port};
    TcpAcceptor acceptor(io, ep);

    boost::asio::ip::udp::endpoint uep{boost::asio::ip::udp::v4(), port};
    UdpSocket udp(io, uep);

    spdlogManager.init();
    net.start();
    acceptor.startAccept();
    udp.startReceive();

    SPDLOG_LOGGER_INFO(logName_, "Echo server listening on port {}...", port);

    while (!g_shouldStop) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    acceptor.stop();
    udp.stop();
    net.stop();
    SPDLOG_LOGGER_INFO(logName_, "Stopped", port);
    return 0;
  } catch (const std::exception &ex) {
    SPDLOG_LOGGER_ERROR(logName_, "{}", ex.what());
    return 1;
  }
}
