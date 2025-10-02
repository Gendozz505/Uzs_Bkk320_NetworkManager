#include "SpdlogManager.hpp"
#include <fstream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

SpdlogManager::SpdlogManager(const std::string &logLevel)
    : logLevel_(logLevel) {

  if (!logName_) {
    logName_ = spdlog::stdout_color_mt("SpdlogManager");
  }
}

void SpdlogManager::init() {

  // Pattern with module name [%n]
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

  static const std::unordered_map<std::string, spdlog::level::level_enum>
      levelMap = {{"trace", spdlog::level::trace},
                  {"debug", spdlog::level::debug},
                  {"info", spdlog::level::info},
                  {"warn", spdlog::level::warn},
                  {"error", spdlog::level::err},
                  {"critical", spdlog::level::critical},
                  {"off", spdlog::level::off}};

  auto it = levelMap.find(logLevel_);
  if (it != levelMap.end()) {
    SPDLOG_LOGGER_INFO(logName_, "Spdlog level = \"{}\"", logLevel_);
    spdlog::set_level(it->second);
  } else {
    SPDLOG_LOGGER_WARN(logName_, "Unknown log level '{}', fallback to info", logLevel_);
    spdlog::set_level(spdlog::level::info);
  }
}
