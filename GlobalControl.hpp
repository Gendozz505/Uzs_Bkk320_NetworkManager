#pragma once

#include "SpdlogManager.hpp"
#include "TcpAcceptor.hpp"
#include "UdpSocket.hpp"
#include "DataParser.hpp"
#include "MessageManager.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <boost/program_options.hpp>

class GlobalControl {
public:
  GlobalControl();
  ~GlobalControl();
  int start(boost::program_options::variables_map &vm);

private:

  // Bind signals to the appropriate handlers
  void bindSignals_();

  boost::asio::io_context io_;

  SpdlogManager spdlog_;
  std::unique_ptr<TcpAcceptor> tcpAcceptor_;
  std::unique_ptr<UdpSocket> udpSocket_;
  std::unique_ptr<DataParser> dataParser_;
  std::unique_ptr<MessageManager> messageManager_;
};