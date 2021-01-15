#include <optional>
#include <vector>

#include "operation_types.hpp"
#include "query_manager.hpp"
#include "query_scheduling_data.hpp"

auto main() -> int {
  query_scheduling_data::QueryNode filtering_query_once = {
      {"CAR_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      operation_types::QueryOperation::kFilter};

  query_scheduling_data::QueryNode pass_through_1k_data = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  query_scheduling_data::QueryNode pass_through_small_data = {
      {"CAR_FILTER_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  query_scheduling_data::QueryNode pass_through_500_data = {
      {"CUSTOMER_DATA.csv"},
      {"CUSTOMER_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  query_scheduling_data::QueryNode pass_through_tpch_data = {
      {"tpch_orders_small.csv"},
      {"tpch_orders_small.csv"},
      operation_types::QueryOperation::kPassThrough};

  query_scheduling_data::QueryNode join_query_once = {
      {"CAR_DATA.csv", "CUSTOMER_DATA_FOR_JOIN.csv"},
      {"JOIN_DATA.csv"},
      operation_types::QueryOperation::kJoin};

  query_scheduling_data::QueryNode tpch_join_once = {
      {"tpch_customer.csv", "tpch_orders_join.csv"},
      {"tpch_customer_orders.csv"},
      operation_types::QueryOperation::kJoin};

  query_scheduling_data::QueryNode merge_sort_query_8k_once_double = {
      {"CAR_DATA_HALF_SORTED_8K_64WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::kMergeSort};

  query_scheduling_data::QueryNode linear_sort_query_8k_once = {
      {"CAR_DATA_8K.csv"},
      {"CAR_DATA_HALF_SORTED_8K_512WAY.csv"},
      operation_types::QueryOperation::kLinearSort};

  // Temp not supported
  query_scheduling_data::QueryNode merge_sort_query_8k_once = {
      {"CAR_DATA_HALF_SORTED_8K_128WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::kMergeSort};
  query_scheduling_data::QueryNode merge_sort_query_1k_once = {
      {"CAR_DATA_HALF_SORTED.csv"},
      {"CAR_DATA_SORTED.csv"},
      operation_types::QueryOperation::kMergeSort};

  query_scheduling_data::QueryNode pass_through_and_filter_query;
  query_scheduling_data::QueryNode filter_after_pass_through_query;

  pass_through_and_filter_query.input_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.output_data_definition_files = {"CAR_DATA.csv"};
  pass_through_and_filter_query.operation_type =
      operation_types::QueryOperation::kPassThrough;
  pass_through_and_filter_query.next_nodes = {&filter_after_pass_through_query};

  filter_after_pass_through_query.input_data_definition_files = {
      "CAR_DATA.csv"};
  filter_after_pass_through_query.output_data_definition_files = {
      "CAR_FILTER_DATA.csv"};
  filter_after_pass_through_query.operation_type =
      operation_types::QueryOperation::kFilter;
  filter_after_pass_through_query.previous_nodes = {
      &pass_through_and_filter_query};

  // Run operations twice
  QueryManager::RunQueries({filtering_query_once, filtering_query_once,
                            merge_sort_query_8k_once_double,
                            merge_sort_query_8k_once_double, join_query_once,
                            join_query_once/*, linear_sort_query_8k_once,
                            linear_sort_query_8k_once*/});
  // Run operations with pass through data
  QueryManager::RunQueries({pass_through_tpch_data, pass_through_500_data,
                            pass_through_small_data, /*pass_through_1k_data,*/
                            join_query_once, merge_sort_query_8k_once_double/*,
                            linear_sort_query_8k_once*/, filtering_query_once});
  return 0;
}
