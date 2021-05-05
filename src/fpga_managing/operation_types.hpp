#pragma once

#include <tuple>
#include <vector>

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Accelerated operation types are listed here.
 */
namespace operation_types {

/**
 * @brief Enumeration for supported accelerated operation types.
 */
enum class QueryOperationType {
  kFilter,  // DNF, CMP
  kJoin,
  kMergeSort,  // Max channel count, channels per module
  kPassThrough,
  kLinearSort,  // Sorted sequence length
  kAddition,
  kMultiplication,
  kAggregationSum
};

/**
 * @brief Tie operation type information together with resource elasticity data
 * to differentiate different modules accelerating the same operation
 */
struct QueryOperation {
  QueryOperationType operation_type;
  std::vector<int> resource_elasticity_data;

  auto operator==(const QueryOperation& rhs) const -> bool {
    return operation_type == rhs.operation_type &&
           resource_elasticity_data == rhs.resource_elasticity_data;
  }

  auto operator<(const QueryOperation& comparable) const -> bool {
    return std::tie(operation_type, resource_elasticity_data) <
           std::tie(comparable.operation_type,
                    comparable.resource_elasticity_data);
  }

  // QueryOperation(QueryOperationType operation_type)
  //    : operation_type{operation_type}, resource_elasticity_data{} {}
  QueryOperation(QueryOperationType operation_type,
                 std::vector<int> resource_elasticity_data)
      : operation_type{operation_type},
        resource_elasticity_data{resource_elasticity_data} {}
};

}  // namespace operation_types

}  // namespace fpga_managing
}  // namespace dbmstodspi