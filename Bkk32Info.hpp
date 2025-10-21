#pragma once

#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Common.hpp"

// #define BKK32_MAIN_CFG_FILE "/opt/Sensor-M/Bkk320/etc/MainCfg.json"
// #define BKK32_MAIN_CFG_FILE "/home/genadi/Uzs_Bkk320_NetworkManager/MainCfg.json"
#define BKK32_MAIN_CFG_FILE "/home/genadi/Projects/Uzs_Bkk320_NetworkManager/Uzs_Bkk320_NetworkManager/MainCfg.json"

using json = nlohmann::json;

inline uint16_t getSerialNumber() {
    try {    
        json j;
        int serialNumber;
    
        std::ifstream file(BKK32_MAIN_CFG_FILE);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
    
        file >> j;
    
        serialNumber = j["SerNumb"];

        return serialNumber;
    } catch (const std::exception& e) {
        spdlog::error("[Bkk32Info] Failed to get serial number: {}", e.what());
        return 0;
    }

}