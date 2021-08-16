/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <tuple>
#include <utility>
#include <vector>

namespace dbmstodspi::fpga_managing::operation_types {

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
        resource_elasticity_data{std::move(resource_elasticity_data)} {}
};

}  // namespace dbmstodspi::fpga_managing::operation_types