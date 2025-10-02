#pragma once

#include "Common.hpp"

#include <spdlog/spdlog.h>
#include <string>

class SpdlogManager {
public:
  SpdlogManager();
  void init();

private:
  int logLevel_;
  std::shared_ptr<spdlog::logger> logName_;
};
