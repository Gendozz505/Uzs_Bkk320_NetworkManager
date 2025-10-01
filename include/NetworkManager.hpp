#pragma once

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <thread>
#include <vector>

class NetworkManager {
public:
    explicit NetworkManager(std::size_t numThreads = std::thread::hardware_concurrency());
    ~NetworkManager();

    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void start();
    void stop();

    boost::asio::io_context& getIoContext();

private:
    boost::asio::io_context ioContext_;
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    std::unique_ptr<WorkGuard> workGuard_;
    std::vector<std::thread> threads_;
    std::size_t numThreads_;
    std::shared_ptr<spdlog::logger> logName_;
};


