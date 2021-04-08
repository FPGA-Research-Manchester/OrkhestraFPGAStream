#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

#include "operation_types.hpp"
#include "query_manager.hpp"
#include "query_scheduling_data.hpp"

using namespace dbmstodspi;

/**
 * @brief Helper method to run the given query nodes and their subsequent nodes
 * while measuring and printing the overall time it took to process the queries.
 *
 * This includes data writing and reading from and to the DDR.
 * @param leaf_nodes Vector of nodes from which the parsing starts.
 */
void MeasureOverallTime(
    std::vector<query_managing::query_scheduling_data::QueryNode> leaf_nodes) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  query_managing::QueryManager::RunQueries(leaf_nodes);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout
      << "Overall time = "
      << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count()
      << "[s]" << std::endl;
}

/**
 * @brief Main method of the program.
 *
 * Creates query nodes to be processed and runs the
 * query nodes in different runs meant to test different things like the module
 * correctness and TPC-H support.
 */
auto main() -> int {
  // CAR DATA
  query_managing::query_scheduling_data::QueryNode pass_through_1k_data = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      fpga_managing::operation_types::QueryOperation::kPassThrough,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {2}}, {}}};
  query_managing::query_scheduling_data::QueryNode filtering_query_once = {
      {"CAR_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      fpga_managing::operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {2}}, {{0}}}};
  query_managing::query_scheduling_data::QueryNode join_query_once = {
      {"CAR_DATA.csv", "CUSTOMER_DATA.csv"},
      {"JOIN_DATA.csv"},
      fpga_managing::operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {0, -1, 1, 2, 3, 4, 5, 6}}, {{}, {2}}, {{2}}}};
  query_managing::query_scheduling_data::QueryNode linear_sort_query_8k_once = {
      {"CAR_DATA_8K.csv"},
      {"CAR_DATA_HALF_SORTED_8K_512WAY.csv"},
      fpga_managing::operation_types::QueryOperation::kLinearSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {2}}, {}}};
  query_managing::query_scheduling_data::QueryNode merge_sort_query_1k_once = {
      {"CAR_DATA_HALF_SORTED.csv"},
      {"CAR_DATA_SORTED.csv"},
      fpga_managing::operation_types::QueryOperation::kMergeSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {2}}, {{64, 16}}}};
  query_managing::query_scheduling_data::QueryNode merge_sort_query_8k_once = {
      {"CAR_DATA_HALF_SORTED_8K_128WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      fpga_managing::operation_types::QueryOperation::kMergeSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {2}}, {{64, 128}}}};
  // Temp not supported
  // query_managing::query_scheduling_data::QueryNode
  // merge_sort_query_8k_once_double = {
  //    {"CAR_DATA_HALF_SORTED_8K_64WAY.csv"},
  //    {"CAR_DATA_SORTED_8K.csv"},
  //    fpga_managing::operation_types::QueryOperation::kMergeSort,
  //    {nullptr},
  //    {nullptr}};

  // PIPELINED QUERY NODES
  query_managing::query_scheduling_data::QueryNode
      pass_through_and_filter_query;
  query_managing::query_scheduling_data::QueryNode
      filter_after_pass_through_query;

  pass_through_and_filter_query.input_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.output_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.operation_type =
      fpga_managing::operation_types::QueryOperation::kPassThrough;
  pass_through_and_filter_query.next_nodes = {&filter_after_pass_through_query};
  pass_through_and_filter_query.previous_nodes = {nullptr};
  pass_through_and_filter_query.operation_parameters = {{{}}, {{}, {2}}, {}};

  filter_after_pass_through_query.input_data_definition_files = {
      "CAR_DATA.csv"};
  filter_after_pass_through_query.output_data_definition_files = {
      "CAR_FILTER_DATA.csv"};
  filter_after_pass_through_query.operation_type =
      fpga_managing::operation_types::QueryOperation::kFilter;
  filter_after_pass_through_query.previous_nodes = {
      &pass_through_and_filter_query};
  filter_after_pass_through_query.next_nodes = {nullptr};
  filter_after_pass_through_query.operation_parameters = {
      {{}}, {{}, {2}}, {{0}}};

  query_managing::query_scheduling_data::QueryNode filter_and_join_query;
  query_managing::query_scheduling_data::QueryNode join_after_filter_query;

  filter_and_join_query.input_data_definition_files = {"CAR_DATA.csv"};
  filter_and_join_query.output_data_definition_files = {"CAR_FILTER_DATA.csv"};
  filter_and_join_query.operation_type =
      fpga_managing::operation_types::QueryOperation::kFilter;
  filter_and_join_query.next_nodes = {&join_after_filter_query};
  filter_and_join_query.previous_nodes = {nullptr};
  filter_and_join_query.operation_parameters = {{{}}, {{}, {2}}, {{0}}};

  join_after_filter_query.input_data_definition_files = {"CAR_FILTER_DATA.csv",
                                                         "CUSTOMER_DATA.csv"};
  join_after_filter_query.output_data_definition_files = {
      "FILTERED_JOIN_DATA.csv"};
  join_after_filter_query.operation_type =
      fpga_managing::operation_types::QueryOperation::kJoin;
  join_after_filter_query.previous_nodes = {&filter_and_join_query, nullptr};
  join_after_filter_query.next_nodes = {nullptr};
  join_after_filter_query.operation_parameters = {
      {{}, {0, -1, 1, 2, 3, 4, 5, 6}}, {{}, {2}}, {{2}}};

  // TPC-H DATA
  // Lineitem tables in different sizes
  query_managing::query_scheduling_data::QueryNode
      tpch_pass_through_lineitem_001 = {
          {"lineitem_sf0_01.csv"},
          {"lineitem_sf0_01.csv"},
          fpga_managing::operation_types::QueryOperation::kPassThrough,
          {nullptr},
          {nullptr},
          {{{}}, {{}, {3}}, {}}};
  query_managing::query_scheduling_data::QueryNode
      tpch_pass_through_lineitem_01 = {
          {"lineitem_sf0_1.csv"},
          {"lineitem_sf0_1.csv"},
          fpga_managing::operation_types::QueryOperation::kPassThrough,
          {nullptr},
          {nullptr},
          {{{}}, {{}, {3}}, {}}};
  // query_managing::query_scheduling_data::QueryNode
  // tpch_pass_through_lineitem_02 = {
  //    {"lineitem_sf0_2.csv"},
  //    {"lineitem_sf0_2.csv"},
  //    fpga_managing::operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // query_managing::query_scheduling_data::QueryNode
  // tpch_pass_through_lineitem_03 = {
  //    {"lineitem_sf0_3.csv"},
  //    {"lineitem_sf0_3.csv"},
  //    fpga_managing::operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // Need optimisations to enable
  // query_managing::query_scheduling_data::QueryNode
  // tpch_pass_through_lineitem_04 = {
  //    {"lineitem_sf0_4.csv"},
  //    {"lineitem_sf0_4.csv"},
  //    fpga_managing::operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};
  // query_managing::query_scheduling_data::QueryNode
  // tpch_pass_through_lineitem_05 = {
  //    {"lineitem_sf0_5.csv"},
  //    {"lineitem_sf0_5.csv"},
  //    fpga_managing::operation_types::QueryOperation::kPassThrough,
  //    {nullptr},
  //    {nullptr}};

  // Q19
  // Input: l_partkey, l_quantity, l_extendedprice, l_discount, l_shipinstruct,
  // l_shipmode Output: l_partkey, l_quantity, l_extendedprice, l_discount
  query_managing::query_scheduling_data::QueryNode first_lineitem_filter = {
      {"lineitem_sf0_01.csv"},
      {"lineitem_sf0_01_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{1,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1,
         -1, -1, -1, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}},
       {{0, 1, 2, 3, 4, 5, 6}, {2}},
       {{1}}}};
  query_managing::query_scheduling_data::QueryNode first_lineitem_filter1 = {
      {"lineitem_sf0_1.csv"},
      {"lineitem_sf0_1_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{1,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1,
         -1, -1, -1, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}},
       {{0, 1, 2, 3, 4, 5, 6}, {2}},
       {{1}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_linear_sort = {
      {"lineitem_sf0_01_filter.csv"},
      {"lineitem_sf0_01_linear_sort.csv"},
      fpga_managing::operation_types::QueryOperation::kLinearSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {1}}, {{}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_linear_sort1 = {
      {"lineitem_sf0_1_filter.csv"},
      {"lineitem_sf0_1_linear_sort.csv"},
      fpga_managing::operation_types::QueryOperation::kLinearSort,
      {nullptr},
      {nullptr},
      {{{}}, {{}, {1}}, {{}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_linear_merge_sort =
      {{"lineitem_sf0_01_linear_sort.csv"},
       {"lineitem_sf0_01_sort.csv"},
       fpga_managing::operation_types::QueryOperation::kMergeSort,
       {nullptr},
       {nullptr},
       {{{}}, {{}, {1}}, {{64, 512}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_linear_merge_sort1 =
      {{"lineitem_sf0_1_linear_sort.csv"},
       {"lineitem_sf0_1_sort.csv"},
       fpga_managing::operation_types::QueryOperation::kMergeSort,
       {nullptr},
       {nullptr},
       {{{}}, {{}, {1}}, {{64, 512}}}};
  query_managing::query_scheduling_data::QueryNode first_part_filter = {
      {"part_sf0_01.csv"},
      {"part_sf0_01_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{0,  22, 23, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, 32, 33, 34, 35, 33, 34, 35, 33, 34, 35}},
       {{0, 1, 2, 3}, {2}},
       {{2}}}};
  query_managing::query_scheduling_data::QueryNode first_part_filter1 = {
      {"part_sf0_1.csv"},
      {"part_sf0_1_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kFilter,
      {nullptr},
      {nullptr},
      {{{0,  22, 23, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, 32, 33, 34, 35, 33, 34, 35, 33, 34, 35}},
       {{0, 1, 2, 3}, {2}},
       {{2}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_part_join = {
      {"lineitem_sf0_01_sort.csv", "part_sf0_01_filter.csv"},
      {"lineitem_part_sf0_01_1st_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {0, -1, -1, -1, -1, -1, -1, 1, 2, 3}},
       {{1, 2, 3, 4, 5, 6, 7, 8, 9}, {1}},
       {{7}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_part_join1 = {
      {"lineitem_sf0_1_sort.csv", "part_sf0_1_filter.csv"},
      {"lineitem_part_sf0_1_1st_filter.csv"},
      fpga_managing::operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {0, -1, -1, -1, -1, -1, -1, 1, 2, 3}},
       {{1, 2, 3, 4, 5, 6, 7, 8, 9}, {1}},
       {{7}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_part_second_filter =
      {{"lineitem_part_sf0_01_1st_filter.csv"},
       {"lineitem_part_sf0_01_2nd_filter.csv"},
       fpga_managing::operation_types::QueryOperation::kFilter,
       {nullptr},
       {nullptr},
       {{{0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1}}, {{2, 3, 4, 5}, {1}}, {{3}}}};
  query_managing::query_scheduling_data::QueryNode
      lineitem_part_second_filter1 = {
          {"lineitem_part_sf0_1_1st_filter.csv"},
          {"lineitem_part_sf0_1_2nd_filter.csv"},
          fpga_managing::operation_types::QueryOperation::kFilter,
          {nullptr},
          {nullptr},
          {{{0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1}}, {{2, 3, 4, 5}, {1}}, {{3}}}};

  // Query nodes for triple bitstream
  query_managing::query_scheduling_data::QueryNode lineitem_linear_merge_sort3 =
      {{"lineitem_sf0_1_linear_sort.csv"},
       {"lineitem_sf0_1_sort_triple.csv"},
       fpga_managing::operation_types::QueryOperation::kMergeSort,
       {nullptr},
       {nullptr},
       {{{0, 1, 2, 1, 2, 3, 4, 5, 6}}, {{}, {1}}, {{64, 512}}}};
  query_managing::query_scheduling_data::QueryNode lineitem_part_join3 = {
      {"lineitem_sf0_1_sort_triple.csv", "part_sf0_1.csv"},
      {"lineitem_part_sf0_1_1st_filter_triple.csv"},
      fpga_managing::operation_types::QueryOperation::kJoin,
      {nullptr},
      {nullptr, nullptr},
      {{{}, {0,  -1, -1, -1, -1, -1, -1, -1, -1, 22, 23, 24, -1,
             -1, -1, -1, 32, 33, 34, 35, 33, 34, 35, 33, 34, 35}},
       {{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, -1,
         -1, -1, -1, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25},
        {2}},
       {{9}}}};
  query_managing::query_scheduling_data::QueryNode
      lineitem_part_second_filter3 = {
          {"lineitem_part_sf0_1_1st_filter_triple.csv"},
          {"lineitem_part_sf0_1_2nd_filter.csv"},
          fpga_managing::operation_types::QueryOperation::kFilter,
          {nullptr},
          {nullptr},
          {{{}}, {{5, 6, 7, 8}, {2}}, {{4}}}};

  lineitem_linear_merge_sort3.next_nodes = {&lineitem_part_join3};
  lineitem_part_join3.previous_nodes = {&lineitem_linear_merge_sort3, nullptr};
  lineitem_part_join3.next_nodes = {&lineitem_part_second_filter3};
  lineitem_part_second_filter3.previous_nodes = {&lineitem_part_join3};

  first_lineitem_filter.next_nodes = {&lineitem_linear_sort};
  lineitem_linear_sort.previous_nodes = {&first_lineitem_filter};
  lineitem_linear_sort.next_nodes = {&lineitem_linear_merge_sort};
  lineitem_linear_merge_sort.previous_nodes = {&lineitem_linear_sort};
  lineitem_linear_merge_sort.next_nodes = {&lineitem_part_join};
  first_part_filter.next_nodes = {&lineitem_part_join};
  lineitem_part_join.previous_nodes = {&lineitem_linear_merge_sort,
                                       &first_part_filter};
  lineitem_part_join.next_nodes = {&lineitem_part_second_filter};
  lineitem_part_second_filter.previous_nodes = {&lineitem_part_join};

  first_lineitem_filter1.next_nodes = {&lineitem_linear_sort1};
  lineitem_linear_sort1.previous_nodes = {&first_lineitem_filter1};
  lineitem_linear_sort1.next_nodes = {&lineitem_linear_merge_sort1};
  lineitem_linear_merge_sort1.previous_nodes = {&lineitem_linear_sort1};
  lineitem_linear_merge_sort1.next_nodes = {&lineitem_part_join1};
  first_part_filter1.next_nodes = {&lineitem_part_join1};
  lineitem_part_join1.previous_nodes = {&lineitem_linear_merge_sort1,
                                        &first_part_filter1};
  lineitem_part_join1.next_nodes = {&lineitem_part_second_filter1};
  lineitem_part_second_filter1.previous_nodes = {&lineitem_part_join1};

  // Run operations twice
  // query_managing::QueryManager::RunQueries(
  //    {filtering_query_once, filtering_query_once, merge_sort_query_8k_once,
  //     merge_sort_query_8k_once, join_query_once, join_query_once,
  //     linear_sort_query_8k_once, linear_sort_query_8k_once});
  // Run operations with pass through data - Currently the scheduler doesn't
  // take memory limits into consideration
  // query_managing::QueryManager::RunQueries(
  //    {
  // /*pass_through_1k_data,
  //                          pass_through_1k_data, pass_through_1k_data,*/
  //                          /*join_query_once,*/
  //                          merge_sort_query_8k_once, pass_through_1k_data/*,
  //                          linear_sort_query_8k_once,
  //                          filtering_query_once*/});

  // Individual operation tests with car data
  // MeasureOverallTime({merge_sort_query_1k_once, filtering_query_once,
  //                   join_query_once, linear_sort_query_8k_once});

  // Pipelined tests
  MeasureOverallTime({first_lineitem_filter, first_part_filter});
  MeasureOverallTime({first_lineitem_filter1, first_part_filter1});
  // Triple bitstream test
  /*MeasureOverallTime({lineitem_linear_merge_sort3});*/

  // QueryManager::RunQueries({filter_and_join_query});
  // QueryManager::RunQueries({join_query_once, filter_and_join_query});
  // QueryManager::RunQueries({pass_through_and_filter_query});

  // MeasureOverallTime({tpch_pass_through_lineitem_01});
  // MeasureOverallTime({tpch_pass_through_lineitem_001});

  return 0;
}
