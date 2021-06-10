#pragma once

#include <memory>
#include <string>

#include "config.hpp"
#include "input_config_reader_interface.hpp"
#include "json_reader_interface.hpp"
#include "operation_types.hpp"

using dbmstodspi::fpga_managing::operation_types::QueryOperation;
using dbmstodspi::fpga_managing::operation_types::QueryOperationType;

namespace dbmstodspi::input_managing {
class ConfigCreator {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;
  std::unique_ptr<InputConfigReaderInterface> config_reader_;

  static auto ConvertStringMapToQueryOperations(
      const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                     std::string>& string_map)
      -> std::map<std::vector<QueryOperation>, std::string>;

  static auto ConvertAcceleratorLibraryToModuleLibrary(
      const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                     std::string>& accelerator_library_data)
      -> std::map<fpga_managing::operation_types::QueryOperationType,
                  std::vector<std::vector<int>>>;

 public:
  ConfigCreator(std::unique_ptr<JSONReaderInterface> json_reader,
                std::unique_ptr<InputConfigReaderInterface> config_reader)
      : json_reader_{std::move(json_reader)},
        config_reader_{std::move(config_reader)} {};
  auto GetConfig(const std::string& config_filename) -> Config;
};
}  // namespace dbmstodspi::input_managing