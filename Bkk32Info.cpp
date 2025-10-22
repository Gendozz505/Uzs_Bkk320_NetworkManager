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
    int serialNumber;

    std::ifstream file(getMainCfgPath());
    if (!file.is_open()) {
      spdlog::error("[Bkk32Info] Failed to open file: {}", getMainCfgPath());
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