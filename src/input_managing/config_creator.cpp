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

#include "config_creator.hpp"

#include <algorithm>
#include <vector>

#include "query_scheduling_data.hpp"
#include "table_data.hpp"

using dbmstodspi::data_managing::table_data::kDataTypeNames;
using dbmstodspi::fpga_managing::operation_types::QueryOperation;
using dbmstodspi::input_managing::ConfigCreator;
using dbmstodspi::query_managing::query_scheduling_data::kSupportedFunctions;

auto ConfigCreator::GetConfig(const std::string& config_filename) -> Config {
  std::string configurations_library = "CONFIGURATIONS_LIBRARY";
  std::string memory_requirements = "BITSTREAMS_MEM_REQ";
  std::string data_type_sizes = "DATA_SIZE_CONFIG";
  std::string data_separator = "DATA_SEPARATOR";

  // repo.json is hardcoded for now.

  auto config_values = config_reader_->ParseInputConfig(config_filename);

  auto accelerator_library_data = json_reader_->readAcceleratorLibrary(
      config_values[configurations_library]);

  Config config;
  config.accelerator_library =
      ConvertStringMapToQueryOperations(accelerator_library_data);
  config.module_library =
      ConvertAcceleratorLibraryToModuleLibrary(accelerator_library_data);

  auto string_key_data_sizes =
      json_reader_->readDataSizes(config_values[data_type_sizes]);
  for (const auto& [string_key, size_scale] : string_key_data_sizes) {
    config.data_sizes.insert({kDataTypeNames.at(string_key), size_scale});
  }

  config.required_memory_space =
      json_reader_->readReqMemorySpace(config_values[memory_requirements]);
  config.separator = config_values[data_separator].c_str()[0];
  return config;
}

auto ConfigCreator::ConvertStringMapToQueryOperations(
    const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                   std::string>& string_map)
    -> std::map<std::vector<QueryOperation>, std::string> {
  std::map<std::vector<QueryOperation>, std::string> resulting_map;
  for (const auto& [vector_key, string_value] : string_map) {
    std::vector<QueryOperation> query_vector;
    for (const auto& operation : vector_key) {
      query_vector.emplace_back(kSupportedFunctions.at(operation.first),
                                operation.second);
    }
    resulting_map.insert({query_vector, string_value});
  }
  return resulting_map;
}

auto ConfigCreator::ConvertAcceleratorLibraryToModuleLibrary(
    const std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                   std::string>& accelerator_library_data)
    -> std::map<fpga_managing::operation_types::QueryOperationType,
                std::vector<std::vector<int>>> {
  std::map<fpga_managing::operation_types::QueryOperationType,
           std::vector<std::vector<int>>>
      resulting_map;
  for (const auto& [vector_key, string_value] : accelerator_library_data) {
    for (const auto& operation : vector_key) {
      if (auto search =
              resulting_map.find(kSupportedFunctions.at(operation.first));
          search != resulting_map.end()) {
        for (int param_index = 0; param_index < operation.second.size();
             param_index++) {
          search->second[param_index].push_back(operation.second[param_index]);
        }
      } else {
        std::vector<std::vector<int>> operation_params;
        for (int param_index : operation.second) {
          operation_params.push_back({param_index});
        }
        resulting_map.insert(
            {kSupportedFunctions.at(operation.first), operation_params});
      }
    }
  }

  for (auto& [operation, params] : resulting_map) {
    for (auto& param_vector : params) {
      std::sort(param_vector.begin(), param_vector.end());
      param_vector.erase(std::unique(param_vector.begin(), param_vector.end()),
                         param_vector.end());
    }
  }

  return resulting_map;
}