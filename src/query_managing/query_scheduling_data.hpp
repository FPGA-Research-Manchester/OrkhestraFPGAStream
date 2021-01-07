#pragma once
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"

namespace query_scheduling_data {
struct QueryNode {
  std::vector<std::string> input_data_definition_files;
  std::vector<std::string> output_data_definition_files;
  operation_types::QueryOperation operation_type;
  std::vector<query_scheduling_data::QueryNode *> next_nodes;
  std::vector<query_scheduling_data::QueryNode *> previous_nodes;

  bool operator==(const QueryNode &rhs) const {
    return input_data_definition_files == rhs.input_data_definition_files &&
           output_data_definition_files == rhs.output_data_definition_files &&
           operation_type == rhs.operation_type &&
           next_nodes == rhs.next_nodes && previous_nodes == rhs.previous_nodes;
  }
};

typedef std::multiset<std::pair<operation_types::QueryOperation, int>>
    ConfigurableModuleSet;

const std::map<ConfigurableModuleSet, std::string>
    supported_accelerator_bitstreams = {
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

        /*{{{operation_types::QueryOperation::kMergeSort, 2},
          {operation_types::QueryOperation::kPassThrough, 0}},
         "DSPI_double_merge_sorting"}, not supported at the moment*/

        {{{operation_types::QueryOperation::kMergeSort, 1}},
         "DSPI_merge_sorting"},

        {{{operation_types::QueryOperation::kLinearSort, 1}},
         "DSPI_linear_sorting"},

        {{{operation_types::QueryOperation::kLinearSort, 1},
          {operation_types::QueryOperation::kPassThrough, 0}},
         "DSPI_linear_sorting"}};

// Should be built up from the corresponding_accelerator_bitstreams map
const std::map<operation_types::QueryOperation, std::vector<int>>
    available_modules = {{operation_types::QueryOperation::kFilter, {1}},
                         {operation_types::QueryOperation::kJoin, {1}},
                         {operation_types::QueryOperation::kMergeSort, {1, 2}},
                         {operation_types::QueryOperation::kPassThrough, {0}},
                         {operation_types::QueryOperation::kLinearSort, {1}}};

}  // namespace query_scheduling_data