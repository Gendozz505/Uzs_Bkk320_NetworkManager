#include "SpdlogManager.hpp"
#include <fstream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

SpdlogManager::SpdlogManager() {

  if (!logName_) {
    logName_ = spdlog::stdout_color_mt("SpdlogManager");
  }
}

void SpdlogManager::init() {

  // Pattern with module name [%n]
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
}
