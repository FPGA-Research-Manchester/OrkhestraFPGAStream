#pragma once
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"

namespace dbmstodspi::query_managing::query_scheduling_data {

using fpga_managing::query_acceleration_constants::kModuleSize;

/**
 * @brief Struct for collecting all of the parameter vectors.
 */
struct NodeOperationParameters {
  std::vector<std::vector<int>> input_stream_parameters;
  std::vector<std::vector<int>> output_stream_parameters;
  std::vector<std::vector<int>> operation_parameters;

  auto operator==(const NodeOperationParameters &rhs) const -> bool {
    return input_stream_parameters == rhs.input_stream_parameters &&
           output_stream_parameters == rhs.output_stream_parameters &&
           operation_parameters == rhs.operation_parameters;
  }
};

/**
 * @brief Struct for defining a query node.
 */
struct QueryNode {
  /// Input data files.
  std::vector<std::string> input_data_definition_files;
  /// Expected data files.
  std::vector<std::string> output_data_definition_files;
  /// Query operation.
  fpga_managing::operation_types::QueryOperationType operation_type;
  /// Pointers to the next query nodes.
  std::vector<query_scheduling_data::QueryNode *> next_nodes;
  /// Pointers to the prerequisite query nodes
  std::vector<query_scheduling_data::QueryNode *> previous_nodes;
  /// Operation parameters to configure the streams with modules.
  NodeOperationParameters operation_parameters;
  /// Location of the module to be processing this node
  int module_location = -1;

  auto operator==(const QueryNode &rhs) const -> bool {
    return input_data_definition_files == rhs.input_data_definition_files &&
           output_data_definition_files == rhs.output_data_definition_files &&
           operation_type == rhs.operation_type &&
           next_nodes == rhs.next_nodes &&
           previous_nodes == rhs.previous_nodes &&
           operation_parameters == rhs.operation_parameters /*&&
           module_location == rhs.module_location*/
        ;  // Last comparison should be included once scheduler has been fixed
           // to work with smart pointers
  }
};

/// Type definition of a collection of operation types for selecting bitstreams.
using ConfigurableModulesVector =
    std::vector<fpga_managing::operation_types::QueryOperation>;

const std::map<fpga_managing::operation_types::QueryOperationType,
               std::vector<std::vector<int>>>
    kExistingModules = {
        {fpga_managing::operation_types::QueryOperationType::kFilter,
         {{8, 16, 32}, {1, 2, 4}}},
        {fpga_managing::operation_types::QueryOperationType::kJoin, {}},
        {fpga_managing::operation_types::QueryOperationType::kMergeSort,
         {{64}}},
        {fpga_managing::operation_types::QueryOperationType::kLinearSort,
         {{512, 1024}}},
        {fpga_managing::operation_types::QueryOperationType::kAddition, {}},
        {fpga_managing::operation_types::QueryOperationType::kMultiplication,
         {}},
        {fpga_managing::operation_types::QueryOperationType::kAggregationSum,
         {}}};

/// Map of supported collections of operations.
const std::map<ConfigurableModulesVector, std::string>
    kSupportedAcceleratorBitstreams = {
        {{{fpga_managing::operation_types::QueryOperationType::kFilter, {32, 4}}}, "DSPI_filtering"},

        {{{fpga_managing::operation_types::QueryOperationType::kJoin, {}}},
         "DSPI_joining"},

        /*{{{fpga_managing::operation_types::QueryOperationType::kMergeSort, {64}},{
          fpga_managing::operation_types::QueryOperationType::kMergeSort, {64}}},
         "DSPI_double_merge_sorting"},*/

        {{{fpga_managing::operation_types::QueryOperationType::kMergeSort,
           {64}}},
         "DSPI_merge_sorting"},

        {{{fpga_managing::operation_types::QueryOperationType::kLinearSort, {512}}}, "DSPI_linear_sorting"},

        {{{fpga_managing::operation_types::QueryOperationType::kAddition, {}}}, "DSPI_addition"},

        {{{fpga_managing::operation_types::QueryOperationType::kMultiplication, {}}}, "DSPI_multiplication"},

        {{{fpga_managing::operation_types::QueryOperationType::kAggregationSum, {}}}, "DSPI_aggregation_sum"},

        {{}, "DSPI_empty"},

        //{{{fpga_managing::operation_types::QueryOperationType::kMergeSort,{64}},
        //  {fpga_managing::operation_types::QueryOperationType::kJoin,{}}, 
        //  {fpga_managing::operation_types::QueryOperationType::kFilter,{32, 4}}},
        // "DSPI_sort_join_filter"},
        // // Needed to find the triple bitstream
        // {{{fpga_managing::operation_types::QueryOperationType::kMergeSort},{},
        //  {fpga_managing::operation_types::QueryOperationType::kJoin},{}},
        // "nonsense"},

        // DNF 8 CMP 1 
        /*{{{fpga_managing::operation_types::QueryOperationType::kFilter,{8,1}},
          {fpga_managing::operation_types::QueryOperationType::kJoin,{}}},
         "DSPI_filter_join"},*/

        // DNF 16 CMP 2 + 1024 way sort
        /*{{{fpga_managing::operation_types::QueryOperationType::kFilter,{16,2}},
          fpga_managing::operation_types::QueryOperationType::kLinearSort{1024}},
         "DSPI_filtering_linear_sort"}*/};

const std::map<std::string, int> kRequiredBitstreamMemorySpace = {
    {"DSPI_filtering", kModuleSize * 2},
    {"DSPI_joining", kModuleSize * 2},
    {"DSPI_double_merge_sorting", kModuleSize * 3},
    {"DSPI_merge_sorting", kModuleSize * 2},
    {"DSPI_linear_sorting", kModuleSize * 2},
    {"DSPI_addition", kModuleSize * 2},
    {"DSPI_multiplication", kModuleSize * 2},
    {"DSPI_aggregation_sum", kModuleSize * 2},
    {"DSPI_empty", kModuleSize * 1},
    {"DSPI_sort_join_filter", kModuleSize * 4},
    {"DSPI_filter_join", kModuleSize * 3},
    {"DSPI_filtering_linear_sort", kModuleSize * 3}
    //{"bitstream containing ILA", kModuleSize * 146}
};

}  // namespace dbmstodspi::query_managing::query_scheduling_data