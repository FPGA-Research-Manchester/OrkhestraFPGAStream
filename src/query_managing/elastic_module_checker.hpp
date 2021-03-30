#pragma once

#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"

class ElasticModuleChecker {
 public:
  static void CheckElasticityNeeds(
      std::vector<StreamDataParameters> input_stream_parameters,
      operation_types::QueryOperation operation_type,
      std::vector<std::vector<int>> operation_parameters,
      query_scheduling_data::ConfigurableModulesVector loaded_modules);

 private:
  static auto IsMergeSortBigEnough(
      std::vector<StreamDataParameters> input_stream_parameters,
      std::vector<std::vector<int>> operation_parameters) -> bool;
};