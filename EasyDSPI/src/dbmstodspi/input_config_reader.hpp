#pragma once

#include "input_config_reader_interface.hpp"

namespace easydspi::dbmstodspi {

class InputConfigReader : public InputConfigReaderInterface {
 public:
  ~InputConfigReader() override = default;
  std::map<std::string, std::string> ParseInputConfig(
      const std::string& filename) override;
};

}  // namespace easydspi::dbmstodspi