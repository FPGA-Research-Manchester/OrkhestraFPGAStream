/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
/**
 * @brief Factory class creating configs based on the config and json readers given.
*/
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
  /**
   * @brief Factory constructor.
   * @param json_reader Object to read JSON files with.
   * @param config_reader Object to read INI files with.
  */
  ConfigCreator(std::unique_ptr<JSONReaderInterface> json_reader,
                std::unique_ptr<InputConfigReaderInterface> config_reader)
      : json_reader_{std::move(json_reader)},
        config_reader_{std::move(config_reader)} {};
  /**
   * @brief Get information out of the given INI file.
   * @param config_filename Filename of the INI file.
   * @return Config struct containing the information in the file.
  */
  auto GetConfig(const std::string& config_filename) -> Config;
};
}  // namespace dbmstodspi::input_managing