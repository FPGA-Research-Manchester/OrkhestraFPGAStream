#pragma once

#include <string>
#include <vector>

namespace easydspi::core {
class Core {
 public:
  static std::vector<std::string> run(std::string input_filename,
                                      std::string config_filename);
};
}  // namespace easydspi::core
