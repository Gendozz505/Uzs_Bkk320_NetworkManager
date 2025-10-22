#pragma once

#include <string>
#include <mutex>

#define BKK32_DEFAULT_MAIN_CFG_PATH "/opt/Sensor-M/Bkk320/data/MainCfg.saved.json"

class Bkk32Info {
public:
  void setMainCfgPath(const std::string &path);
  std::string getMainCfgPath();
  uint16_t getSerialNumber();
  
  private:
  std::mutex mainCfgMutex_;
  std::string mainCfgPath_;
};