#include "SpdlogManager.hpp"
#include <string>

SpdlogManager::SpdlogManager() {}
SpdlogManager::~SpdlogManager() {}

void SpdlogManager::init(std::string &logLevel) {

  logLevel_ = logLevel;

  // Pattern with module name [%n]
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

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
    spdlog::info("Spdlog level = \"{}\"", logLevel_);
    spdlog::set_level(it->second);
  } else {
    spdlog::info("Unknown log level '{}', fallback to info", logLevel_);
    spdlog::set_level(spdlog::level::info);
  }
}
