#pragma once

#include <string>
#include <spdlog/spdlog.h>

#define SPDLOG_LEVEL "debug" // Can be overridden by the Makefile

class SpdlogManager {
public:
  SpdlogManager(const std::string& logLevel);
  void init();

private:
    std::string logLevel_;
    std::shared_ptr<spdlog::logger> logName_;
};
