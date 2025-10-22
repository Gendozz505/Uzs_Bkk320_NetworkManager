#include "Bkk32Info.hpp"
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

void Bkk32Info::setMainCfgPath(const std::string &path) {
  mainCfgPath_ = path;
}

std::string Bkk32Info::getMainCfgPath() {
  return mainCfgPath_;
}

uint16_t Bkk32Info::getSerialNumber() {
  try {
    json j;
    uint16_t serialNumber;

    std::ifstream file(mainCfgPath_);
    if (!file.is_open()) {
      spdlog::error("[Bkk32Info] Failed to open file: {}", mainCfgPath_);
      return 0;
    }

    file >> j;

    serialNumber = j["SerNumb"];

    return serialNumber;
  } catch (const std::exception &e) {
    spdlog::error("[Bkk32Info] Failed to get serial number: {}", e.what());
    return 0;
  }
}