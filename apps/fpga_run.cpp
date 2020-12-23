#include "operation_types.hpp"
#include "query_manager.hpp"
#include "query_node.hpp"

auto main() -> int {
  QueryNode filtering_query_once = {{"CAR_DATA.csv"},
                                    {"CAR_FILTER_DATA.csv"},
                                    operation_types::QueryOperation::kFilter};

  QueryNode pass_through_1k_data = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  // Still need to add support for chained query nodes.
  QueryNode pass_through_and_filter_query = {
      {"CAR_DATA.csv"},
      {"CAR_DATA.csv"},
      operation_types::QueryOperation::kPassThrough,
      &filtering_query_once};

  QueryNode pass_through_small_data = {
      {"CAR_FILTER_DATA.csv"},
      {"CAR_FILTER_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  QueryNode pass_through_500_data = {
      {"CUSTOMER_DATA.csv"},
      {"CUSTOMER_DATA.csv"},
      operation_types::QueryOperation::kPassThrough};

  QueryNode join_query_once = {{"CAR_DATA.csv", "CUSTOMER_DATA_FOR_JOIN.csv"},
                               {"JOIN_DATA.csv"},
                               operation_types::QueryOperation::kJoin};

  QueryNode merge_sort_query_8k_once_double = {
      {"CAR_DATA_HALF_SORTED_8K_64WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::kMergeSort};

  // Temp not supported
  QueryNode merge_sort_query_8k_once = {
      {"CAR_DATA_HALF_SORTED_8K_128WAY.csv"},
      {"CAR_DATA_SORTED_8K.csv"},
      operation_types::QueryOperation::kMergeSort};
  QueryNode merge_sort_query_1k_once = {
      {"CAR_DATA_HALF_SORTED.csv"},
      {"CAR_DATA_SORTED.csv"},
      operation_types::QueryOperation::kMergeSort};

  QueryManager::RunQueries(
      {filtering_query_once, merge_sort_query_8k_once_double, join_query_once});
  return 0;
}
