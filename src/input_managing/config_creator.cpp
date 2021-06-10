#include "config_creator.hpp"

#include <algorithm>
#include <vector>

#include "query_scheduling_data.hpp"

using dbmstodspi::fpga_managing::operation_types::QueryOperation;
using dbmstodspi::input_managing::ConfigCreator;
using dbmstodspi::query_managing::query_scheduling_data::kSupportedFunctions;

auto ConfigCreator::GetConfig(const std::string& config_filename) -> Config {
  std::string configurations_library = "CONFIGURATIONS_LIBRARY";
  std::string driver_selection = "DRIVER_SELECTION";
  std::string memory_requirements = "BITSTREAMS_MEM_REQ";
  std::string data_type_sizes = "DATA_SIZE_CONFIG";

  auto json_file_names = config_reader_->ParseInputConfig(config_filename);

  auto accelerator_library_data = json_reader_->readAcceleratorLibrary(
      json_file_names[configurations_library]);

  Config config;
  config.accelerator_library =
      ConvertStringMapToQueryOperations(accelerator_library_data);
  config.module_library =
      ConvertAcceleratorLibraryToModuleLibrary(accelerator_library_data);

  config.data_sizes =
      json_reader_->readDataSizes(json_file_names[data_type_sizes]);
  config.required_memory_space =
      json_reader_->readReqMemorySpace(json_file_names[memory_requirements]);
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