#include "config_creator.hpp"

#include "query_scheduling_data.hpp"

using dbmstodspi::fpga_managing::operation_types::QueryOperation;
using dbmstodspi::input_managing::ConfigCreator;
using dbmstodspi::query_managing::query_scheduling_data::kSupportedFunctions;

auto ConfigCreator::GetConfig(std::string config_filename) -> Config {
  std::string configurations_library = "CONFIGURATIONS_LIBRARY";
  std::string driver_selection = "DRIVER_SELECTION";
  std::string memory_requirements = "BITSTREAMS_MEM_REQ";
  std::string data_type_sizes = "DATA_SIZE_CONFIG";

  auto json_file_names = config_reader_->ParseInputConfig("config.ini");

  Config config;
  config.accelerator_library =
      ConvertStringMapToQueryOperations(json_reader_->readAcceleratorLibrary(
          json_file_names[configurations_library]));
  // Should be built from the accelerator library!
  config.accelerator_library = dbmstodspi::query_managing::
      query_scheduling_data::kSupportedAcceleratorBitstreams;
  config.module_library =
      dbmstodspi::query_managing::query_scheduling_data::kExistingModules;
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