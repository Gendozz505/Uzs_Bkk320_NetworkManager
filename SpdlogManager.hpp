#pragma once

#include "Common.hpp"

#include <string>

class SpdlogManager {
public:
  SpdlogManager();
  ~SpdlogManager();
  void init(std::string &logLevel);

private:
  std::string logLevel_;
};
