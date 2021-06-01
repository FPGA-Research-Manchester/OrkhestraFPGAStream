#pragma once
#include <map>
#include <string>

namespace easydspi::dbmstodspi {

class InputConfigReaderInterface {
 public:
  virtual ~InputConfigReaderInterface() = default;
  virtual auto ParseInputConfig(const std::string& filename)
      -> std::map<std::string, std::string> = 0;
};

}  // namespace easydspi::dbmstodspi