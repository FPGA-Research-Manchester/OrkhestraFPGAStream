#include "elastic_module_checker.hpp"

#include <stdexcept>

using namespace dbmstodspi::query_managing;

void ElasticModuleChecker::CheckElasticityNeeds(
    const std::vector<fpga_managing::StreamDataParameters>&
        input_stream_parameters,
    fpga_managing::operation_types::QueryOperationType operation_type,
    const std::vector<std::vector<int>>& operation_parameters) {
  if (operation_type ==
          fpga_managing::operation_types::QueryOperationType::kMergeSort &&
      !IsMergeSortBigEnough(input_stream_parameters, operation_parameters)) {
    throw std::runtime_error(
        "Unable to use current merge sort on the given data!");
  }
}

auto ElasticModuleChecker::IsMergeSortBigEnough(
    const std::vector<fpga_managing::StreamDataParameters>&
        input_stream_parameters,
    const std::vector<std::vector<int>>& operation_parameters) -> bool {
  return input_stream_parameters.at(0).stream_record_count <=
         operation_parameters.at(0).at(0) * operation_parameters.at(0).at(1);
}