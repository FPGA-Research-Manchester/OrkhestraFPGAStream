#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

#include "operation_types.hpp"
#include "query_manager.hpp"
#include "query_scheduling_data.hpp"

void MeasureOverallTime(
    std::vector<query_scheduling_data::QueryNode> leaf_nodes) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  QueryManager::RunQueries(leaf_nodes);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout
      << "Overall time = "
      << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count()
      << "[s]" << std::endl;
}

auto main() -> int {
  // CAR DATA
  query_scheduling_data::QueryNode pass_through_1k_data = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::kPassThrough,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  query_scheduling_data::QueryNode filtering_query_once = {
      {"CAR_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {{0}}}};
  query_scheduling_data::QueryNode join_query_once = {
      {"CAR_DATA.csv", "CUSTOMER_DATA.csv"},
      {"JOIN_DATA.csv"},
      operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {0, -1, 1, 2, 3, 4, 5, 6}}, {{}}, {}}};
  query_scheduling_data::QueryNode linear_sort_query_8k_once = {
      {"CAR_DATA_8K.csv"},
      {"CAR_DATA_HALF_SORTED_8K_512WAY.csv"},
      operation_types::QueryOperation::kLinearSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  query_scheduling_data::QueryNode merge_sort_query_1k_once = {
      {"CAR_DATA_HALF_SORTED.csv"},
      {"CAR_DATA_SORTED.csv"},
      operation_types::QueryOperation::kMergeSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  query_scheduling_data::QueryNode merge_sort_query_8k_once = {
      {"CAR_DATA_HALF_SORTED_8K_128WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::kMergeSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  // Temp not supported
  // query_scheduling_data::QueryNode merge_sort_query_8k_once_double = {
  //    {"CAR_DATA_HALF_SORTED_8K_64WAY.csv"},
  //    {"CAR_DATA_SORTED_8K.csv"},
  //    operation_types::QueryOperation::kMergeSort,
  //    {nullptr},
  //    {nullptr}};

  // PIPELINED QUERY NODES
  query_scheduling_data::QueryNode pass_through_and_filter_query;
  query_scheduling_data::QueryNode filter_after_pass_through_query;

  pass_through_and_filter_query.input_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.output_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.operation_type =
      operation_types::QueryOperation::kPassThrough;
  pass_through_and_filter_query.next_nodes = {&filter_after_pass_through_query};
  pass_through_and_filter_query.previous_nodes = {nullptr};
  pass_through_and_filter_query.operation_parameters = {{{}}, {{}}, {}};

  filter_after_pass_through_query.input_data_definition_files = {
      "CAR_DATA.csv"};
  filter_after_pass_through_query.output_data_definition_files = {
      "CAR_FILTER_DATA.csv"};
  filter_after_pass_through_query.operation_type =
      operation_types::QueryOperation::kFilter;
  filter_after_pass_through_query.previous_nodes = {
      &pass_through_and_filter_query};
  filter_after_pass_through_query.next_nodes = {nullptr};
  filter_after_pass_through_query.operation_parameters = {{{}}, {{}}, {{0}}};

  query_scheduling_data::QueryNode filter_and_join_query;
  query_scheduling_data::QueryNode join_after_filter_query;

  filter_and_join_query.input_data_definition_files = {"CAR_DATA.csv"};
  filter_and_join_query.output_data_definition_files = {"CAR_FILTER_DATA.csv"};
  filter_and_join_query.operation_type =
      operation_types::QueryOperation::kFilter;
  filter_and_join_query.next_nodes = {&join_after_filter_query};
  filter_and_join_query.previous_nodes = {nullptr};
  filter_and_join_query.operation_parameters = {{{}}, {{}}, {{0}}};

  join_after_filter_query.input_data_definition_files = {"CAR_FILTER_DATA.csv",
                                                         "CUSTOMER_DATA.csv"};
  join_after_filter_query.output_data_definition_files = {
      "FILTERED_JOIN_DATA.csv"};
  join_after_filter_query.operation_type =
      operation_types::QueryOperation::kJoin;
  join_after_filter_query.previous_nodes = {&filter_and_join_query, nullptr};
  join_after_filter_query.next_nodes = {nullptr};
  join_after_filter_query.operation_parameters = {
      {{}, {0, -1, 1, 2, 3, 4, 5, 6}}, {{}}, {}};

  // TPC-H DATA
  // Lineitem tables in different sizes
  query_scheduling_data::QueryNode tpch_pass_through_lineitem_001 = {
      {"lineitem_sf0_01.csv"},
      {"lineitem_sf0_01.csv"},
      operation_types::QueryOperation::kPassThrough,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  query_scheduling_data::QueryNode tpch_pass_through_lineitem_01 = {
      {"lineitem_sf0_1.csv"},
      {"lineitem_sf0_1.csv"},
      operation_types::QueryOperation::kPassThrough,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {}}};
  // query_scheduling_data::QueryNode tpch_pass_through_lineitem_02 = {
  //    {"lineitem_sf0_2.csv"},
  //    {"lineitem_sf0_2.csv"},
  //    operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // query_scheduling_data::QueryNode tpch_pass_through_lineitem_03 = {
  //    {"lineitem_sf0_3.csv"},
  //    {"lineitem_sf0_3.csv"},
  //    operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // Need optimisations to enable
  // query_scheduling_data::QueryNode tpch_pass_through_lineitem_04 = {
  //    {"lineitem_sf0_4.csv"},
  //    {"lineitem_sf0_4.csv"},
  //    operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // query_scheduling_data::QueryNode tpch_pass_through_lineitem_05 = {
  //    {"lineitem_sf0_5.csv"},
  //    {"lineitem_sf0_5.csv"},
  //    operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};

  // Q19
  query_scheduling_data::QueryNode first_lineitem_filter = {
      {"lineitem_sf0_01.csv"},
      {"lineitem_sf0_01_filter.csv"},
      operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{}}, {{}}, {{1}}}};
  query_scheduling_data::QueryNode lineitem_part_join = {
      {"lineitem_sf0_01_sort.csv", "part_sf0_01_filter_shifted.csv"},
      {"lineitem_part_sf0_01_1st_filter.csv"},
      operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {}}, {{}}, {}}};
  query_scheduling_data::QueryNode first_part_filter = {
      {"part_sf0_01.csv"},
      {"part_sf0_01_filter.csv"},
      operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 33, 34, 35, 33, 34, 35}},
       {{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
         30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43}},
       {{2}}}};

  // Run operations twice
  // QueryManager::RunQueries(
  //    {filtering_query_once, filtering_query_once, merge_sort_query_8k_once,
  //     merge_sort_query_8k_once, join_query_once, join_query_once,
  //     linear_sort_query_8k_once, linear_sort_query_8k_once});
  // Run operations with pass through data - Currently the scheduler doesn't
  // take memory limits into consideration
  // QueryManager::RunQueries({pass_through_1k_data
  // pass_through_1k_data,
  //                          pass_through_1k_data, pass_through_1k_data,
  //                          join_query_once,
  //                          merge_sort_query_8k_once,
  //                          linear_sort_query_8k_once, filtering_query_once});

  // Individual operation tests with car data
  // MeasureOverallTime({merge_sort_query_1k_once, filtering_query_once,
  //                    join_query_once, linear_sort_query_8k_once});

  MeasureOverallTime({first_part_filter});

  // Pipelined tests
  // QueryManager::RunQueries({filter_and_join_query});
  // QueryManager::RunQueries({join_query_once, filter_and_join_query});
  // QueryManager::RunQueries({pass_through_and_filter_query});

  // MeasureOverallTime({tpch_pass_through_lineitem_01});
  // MeasureOverallTime({tpch_pass_through_lineitem_001});

  // MeasureOverallTime({first_lineitem_filter});
  // MeasureOverallTime({first_part_filter});
  // MeasureOverallTime({lineitem_part_join});
  return 0;
}
