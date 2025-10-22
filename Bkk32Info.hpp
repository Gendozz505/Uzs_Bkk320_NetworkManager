#pragma once

#include <string>

#define BKK32_DEFAULT_MAIN_CFG_PATH "/opt/Sensor-M/Bkk320/data/MainCfg.saved.json"

class Bkk32Info {
public:
  void setMainCfgPath(const std::string &path);
  std::string getMainCfgPath();
  uint16_t getSerialNumber();
  
  private:
  std::string mainCfgPath_;
};