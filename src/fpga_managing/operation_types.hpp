#pragma once
/**
 * Accelerated operation types are listed here.
 */
namespace operation_types {
// This type needs to be used in a class to map it to some specific operation
// node. In addition, resource elastic module information needs to be added.

/**
 * Enumeration for supported accelerated operation types.
 */
enum class QueryOperation {
  kFilter,
  kJoin,
  kMergeSort,
  kPassThrough,
  kLinearSort
};
}  // namespace operation_types