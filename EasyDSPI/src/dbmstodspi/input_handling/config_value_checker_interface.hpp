#pragma once

#include "config.hpp"

using easydspi::core_interfaces::Config;

namespace easydspi::dbmstodspi {
class ConfigValueCheckerInterface {
 public:
  virtual ~ConfigValueCheckerInterface() = default;
  virtual bool check(Config input_config) = 0;
};
}  // namespace easydspi::dbmstodspi