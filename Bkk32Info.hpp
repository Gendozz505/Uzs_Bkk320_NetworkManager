#pragma once

#include <string>

#define BKK32_DEFAULT_MAIN_CFG_FILE "/opt/Sensor-M/Bkk320/etc/MainCfg.json"

class Bkk32Info {
public:
  void setMainCfgPath(const std::string &path);
  std::string getMainCfgPath();
  uint16_t getSerialNumber();
  
  private:
  std::string mainCfgPath_;
};