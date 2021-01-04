#pragma once
#include <optional>
#include <utility>
#include <set>
#include <string>
#include <vector>
#include <map>

#include "operation_types.hpp"

namespace query_scheduling_data {
struct QueryNode {
  std::vector<std::string> input_data_definition_files;
  std::vector<std::string> output_data_definition_files;
  operation_types::QueryOperation operation_type;
  std::optional<query_scheduling_data::QueryNode*> next_node;

  bool operator==(const QueryNode &rhs) const {
    return input_data_definition_files == rhs.input_data_definition_files &&
           output_data_definition_files == rhs.output_data_definition_files &&
           operation_type == rhs.operation_type && next_node == rhs.next_node;
  }
};

typedef std::multiset<std::pair<operation_types::QueryOperation, int>>
    ConfigurableModuleSet;

const std::map<ConfigurableModuleSet, std::string>
    corresponding_accelerator_bitstreams = {
        {{{operation_types::QueryOperation::kFilter, 1}}, "DSPI_filtering"},
        {{{operation_types::QueryOperation::kFilter, 1},
          {operation_types::QueryOperation::kPassThrough, 0}},
         "DSPI_filtering"},
        {{{operation_types::QueryOperation::kJoin, 1}}, "DSPI_joining"},
        {{{operation_types::QueryOperation::kJoin, 1},
          {operation_types::QueryOperation::kPassThrough, 0}},
         "DSPI_joining"},
        {{{operation_types::QueryOperation::kMergeSort, 2}},
         "DSPI_double_merge_sorting"},
        {{{operation_types::QueryOperation::kMergeSort, 2},
          {operation_types::QueryOperation::kPassThrough, 0}},
         "DSPI_double_merge_sorting"},
        {{{operation_types::QueryOperation::kMergeSort, 1}},
         "DSPI_merge_sorting"}};

// Should be built up from the corresponding_accelerator_bitstreams map
const std::map<operation_types::QueryOperation, std::vector<int>>
    available_modules = {{operation_types::QueryOperation::kFilter, {1}},
                         {operation_types::QueryOperation::kJoin, {1}},
                         {operation_types::QueryOperation::kMergeSort, {1,2}},
                         {operation_types::QueryOperation::kPassThrough, {0}}};

}  // namespace query_scheduling_data