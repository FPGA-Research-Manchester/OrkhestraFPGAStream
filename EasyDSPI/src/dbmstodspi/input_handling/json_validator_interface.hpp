#pragma once

#include <string>

namespace easydspi::dbmstodspi {
class JSONValidatorInterface {
 public:
  virtual ~JSONValidatorInterface() = default;
  virtual bool check(std::string json_filename) = 0;
};
}  // namespace easydspi::dbmstodspi