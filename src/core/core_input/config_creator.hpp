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
#include "config_value_checker_interface.hpp"
#include "input_config_reader_interface.hpp"
#include "json_reader_interface.hpp"
#include "json_validator_interface.hpp"
#include "operation_types.hpp"

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::operation_types::QueryOperation;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::dbmstodspi::ConfigValueCheckerInterface;
using orkhestrafs::dbmstodspi::InputConfigReaderInterface;
using orkhestrafs::dbmstodspi::JSONReaderInterface;
using orkhestrafs::dbmstodspi::JSONValidatorInterface;

namespace orkhestrafs::core::core_input {
/**
 * @brief Factory class creating configs based on the config files and json
 * readers given.
 */
class ConfigCreator {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;
  std::unique_ptr<InputConfigReaderInterface> config_reader_;
  std::unique_ptr<JSONValidatorInterface> json_validator_;
  std::unique_ptr<ConfigValueCheckerInterface> config_validator_;

  static auto ConvertStringMapToQueryOperations(
      const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                     std::string>& string_map)
      -> std::map<std::vector<QueryOperation>, std::string>;

  static auto ConvertAcceleratorLibraryToModuleLibrary(
      const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                     std::string>& accelerator_library_data)
      -> std::map<QueryOperationType, std::vector<std::vector<int>>>;

  static auto CreateTablesData(
      const std::vector<
          std::map<std::string, std::variant<std::string, int,
                                             std::vector<std::vector<int>>>>>&
          tables_data_in_string_form) -> std::vector<TableMetadata>;

 public:
  /**
   * @brief Factory constructor.
   * @param json_reader Object to read JSON files with.
   * @param config_reader Object to read INI files with.
   * @param json_validator Object to check JSON format.
   * @param config_validator Object to check config validity.
   */
  ConfigCreator(std::unique_ptr<JSONReaderInterface> json_reader,
                std::unique_ptr<InputConfigReaderInterface> config_reader,
                std::unique_ptr<JSONValidatorInterface> json_validator,
                std::unique_ptr<ConfigValueCheckerInterface> config_validator)
      : json_reader_{std::move(json_reader)},
        config_reader_{std::move(config_reader)},
        json_validator_{std::move(json_validator)},
        config_validator_{std::move(config_validator)} {};
  /**
   * @brief Get information out of the given INI file.
   * @param config_filename Filename of the INI file.
   * @return Config struct containing the information in the file.
   */
  auto GetConfig(const std::string& config_filename) -> Config;
};
}  // namespace orkhestrafs::core::core_input