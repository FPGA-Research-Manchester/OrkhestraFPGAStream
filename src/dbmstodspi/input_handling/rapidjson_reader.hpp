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

#include "json_reader_interface.hpp"
#include "rapidjson/document.h"

using orkhestrafs::core_interfaces::JSONReaderInterface;
using rapidjson::Document;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to read JSON files using https://github.com/Tencent/rapidjson/
 */
class RapidJSONReader : public JSONReaderInterface {
 private:
  static auto Read(const std::string& json_filename)
      -> std::unique_ptr<Document>;
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;
  static void GetOperationParameters(
      const rapidjson::GenericMember<
          rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter,
      JSONReaderInterface::InputNodeParameters& node_parameters_map);
  static void GetIOStreamFilesAndDependencies(
      const rapidjson::GenericMember<
          rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter,
      JSONReaderInterface::InputNodeParameters& node_parameters_map);
  static void GetOperationType(
      JSONReaderInterface::InputNodeParameters& node_parameters_map,
      const rapidjson::GenericMember<
          rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter);

 public:
  ~RapidJSONReader() override = default;
  /**
   * @brief Read the JSON file describing a simple map.
   * @param json_filename JSON file.
   * @return Value map.
   */
  auto ReadValueMap(std::string json_filename)
      -> std::map<std::string, double> override;
  /**
   * @brief Read memory mapped register space sizes.
   * @param json_filename JSON file.
   * @return How much space each bitstream has for memory mapped registers.
   */
  auto ReadReqMemorySpace(std::string json_filename)
      -> std::map<std::string, int> override;
  /**
   * @brief Read bitstreams
   * @param json_filename JSON file
   * @return Which module combination is associated with which bitstream name.
   */
  auto ReadAcceleratorLibrary(std::string json_filename)
      -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                  std::string> override;
  /**
   * @brief Read input def - query plan.
   * @param json_filename JSON file
   * @return Return query plan node field names and their values.
   */
  auto ReadInputDefinition(std::string json_filename)
      -> std::map<std::string,
                  JSONReaderInterface::InputNodeParameters> override;

  /**
   * @brief Method to read table meta data.
   * @param json_filename JSON file
   * @return Return table sizes and sorted statuses of currently available
   * tables.
   */
  auto ReadAllTablesData(std::string json_filename)
      -> std::vector<TableMetaDataStringMap> override;

  auto ReadHWLibraryData(std::string json_filename)
      -> HWLibraryStringMap override;

  void WriteValueMap(std::map<std::string, double> data,
                     const std::string& json_filename) override;
};
}  // namespace orkhestrafs::dbmstodspi