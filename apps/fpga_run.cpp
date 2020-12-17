#include "operation_types.hpp"
#include "query_manager.hpp"
#include "query_node.hpp"

auto main() -> int {
  QueryNode filtering_query_once = {{"CAR_DATA.csv"},
                                    {"CAR_FILTER_DATA.csv"},
                                    operation_types::QueryOperation::Filter};

  QueryNode pass_through_1k_data = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::PassThrough};

  // Still need to add support for chained query nodes.
  QueryNode pass_through_and_filter_query = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::PassThrough,
      &filtering_query_once};

  QueryNode pass_through_small_data = {
      {"CAR_FILTER_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      operation_types::QueryOperation::PassThrough};

  QueryNode pass_through_500_data = {
      {"CUSTOMER_DATA.csv"},
      {"CUSTOMER_DATA.csv"},
      operation_types::QueryOperation::PassThrough};

  QueryNode join_query_once = {{"CAR_DATA.csv", "CUSTOMER_DATA_FOR_JOIN.csv"},
                               {"JOIN_DATA.csv"},
                               operation_types::QueryOperation::Join};

  // Temp not supported
  QueryNode merge_sort_query_8k_once = {
      {"CAR_DATA_HALF_SORTED_8K_128WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::MergeSort};

  QueryNode merge_sort_query_1k_once = {
      {"CAR_DATA_HALF_SORTED.csv"},
      {"CAR_DATA_SORTED.csv"},
      operation_types::QueryOperation::MergeSort};
  QueryManager::RunQueries({});
  //QueryManager::RunQueries({filtering_query_once, merge_sort_query_8k_once,
  //                          join_query_once, merge_sort_query_1k_once});
  return 0;
}
