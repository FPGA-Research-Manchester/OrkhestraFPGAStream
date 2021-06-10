#pragma once

#include "input_config_reader_interface.hpp"

namespace dbmstodspi::input_managing {

class InputConfigReader : public InputConfigReaderInterface {
 public:
  ~InputConfigReader() override = default;
  auto ParseInputConfig(const std::string& filename)
      -> std::map<std::string, std::string> override;
};

}  // namespace dbmstodspi::input_managing