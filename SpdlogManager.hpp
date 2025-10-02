#pragma once

#include "Common.hpp"

#include <string>

class SpdlogManager {
public:
  SpdlogManager();
  void init();

private:
  std::string logLevel_;
};
