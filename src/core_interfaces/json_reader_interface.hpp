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

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace orkhestrafs::core_interfaces {
/**
 * @brief Interface for classes to read JSON.
 */
class JSONReaderInterface {
 public:
  /**
   * @brief Datatype for node parameters from the query graph definition file. A
   * node parameter could have a string or a vector of string or more vectors of
   * strings as a value.
   */
  using InputNodeParameters = std::map<
      std::string,
      std::variant<std::string, std::vector<std::string>,
                   std::map<std::string, std::vector<std::vector<int>>>>>;
  /**
   * @brief Datatype for table metadata
   */
  using TableMetaDataStringMap =
      std::map<std::string,
               std::variant<std::string, int, std::vector<int>>>;

  using HWLibraryStringMap = std::map<
      std::string,
      std::pair<std::map<std::string,
                         std::map<std::string, std::variant<std::vector<int>,
                                                            int, std::string>>>,
                std::vector<std::vector<std::string>>>>;

  virtual ~JSONReaderInterface() = default;
  /**
   * @brief Read the JSON file describing mapped values.
   * @param json_filename JSON file.
   * @return Mapped values.
   */
  virtual auto ReadValueMap(std::string json_filename)
      -> std::map<std::string, double> = 0;
  /**
   * @brief Read memory mapped register space sizes.
   * @param json_filename JSON file.
   * @return How much space each bitstream has for memory mapped registers.
   */
  virtual auto ReadReqMemorySpace(std::string json_filename)
      -> std::map<std::string, int> = 0;
  /**
   * @brief Read bitstreams
   * @param json_filename JSON file
   * @return Which module combination is associated with which bitstream name.
   */
  virtual auto ReadAcceleratorLibrary(std::string json_filename)
      -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                  std::string> = 0;
  /**
   * @brief Read input def - query plan.
   * @param json_filename JSON file
   * @return Return query plan node field names and their values.
   */
  virtual auto ReadInputDefinition(std::string json_filename)
      -> std::map<std::string, InputNodeParameters> = 0;

  /**
   * @brief Read tables metadata - How is it sorted and how large is it.
   * @param json_filename JSON file
   * @return Return metadata about all of the current tables usable.
   */
  virtual auto ReadAllTablesData(std::string json_filename)
      -> std::vector<TableMetaDataStringMap> = 0;

  /**
   * @brief Read HW library capacities and module locations data
   * @param json_filename JSON file
   * @return Return a map of the read json.
   */
  virtual auto ReadHWLibraryData(std::string json_filename)
      -> HWLibraryStringMap = 0;

  /**
   * @brief Write value map to a json file.
   * @param data Map of values to be written.
   * @param json_filename JSON file
   */
  virtual void WriteValueMap(std::map<std::string, double> data,
                             const std::string& json_filename) = 0;
};
}  // namespace orkhestrafs::core_interfaces