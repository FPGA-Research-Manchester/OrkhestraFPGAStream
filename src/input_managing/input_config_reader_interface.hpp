#pragma once
#include <map>
#include <string>

namespace dbmstodspi::input_managing {

class InputConfigReaderInterface {
 public:
  virtual ~InputConfigReaderInterface() = default;
  virtual auto ParseInputConfig(const std::string& filename)
      -> std::map<std::string, std::string> = 0;
};

}  // namespace dbmstodspi::input_managing