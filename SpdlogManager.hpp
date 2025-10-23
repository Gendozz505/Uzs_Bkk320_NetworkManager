#pragma once

#include "Common.hpp"

#include <string>

class SpdlogManager {
public:
  SpdlogManager(const std::string &logLevel);
  void init();

private:
  std::string logLevel_;
};
