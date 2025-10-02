#include "NetworkManager.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

NetworkManager::NetworkManager(std::size_t numThreads)
    : workGuard_(), numThreads_(numThreads == 0 ? 1 : numThreads) {
}

NetworkManager::~NetworkManager() { stop(); }

void NetworkManager::start() {
  if (!workGuard_) {
    workGuard_ =
        std::make_unique<WorkGuard>(boost::asio::make_work_guard(ioContext_));
  }
  if (threads_.empty()) {
    threads_.reserve(numThreads_);
    for (std::size_t i = 0; i < numThreads_; ++i) {
      threads_.emplace_back([this]() { ioContext_.run(); });
    }
  }
}

void NetworkManager::stop() {
  if (workGuard_) {
    workGuard_->reset();
    workGuard_.reset();
  }
  ioContext_.stop();
  for (auto &t : threads_) {
    if (t.joinable())
      t.join();
  }
  threads_.clear();
  ioContext_.restart();
}

boost::asio::io_context &NetworkManager::getIoContext() { return ioContext_; }
