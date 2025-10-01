#include "SpdlogManager.hpp"
#include <fstream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

SpdlogManager::SpdlogManager(const std::string &logLevel)
    : logLevel_(logLevel) {}

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
    spdlog::set_level(it->second);
  } else {
    spdlog::set_level(spdlog::level::info);
    spdlog::warn("Unknown log level '{}', fallback to info", logLevel_);
  }
}
