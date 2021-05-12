#pragma once
#include <vector>

#include "accelerated_query_node.hpp"
#include "operation_types.hpp"
#include "stream_data_parameters.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Struct to hold all of the important information about a query node to
 * accelerate it with an FPGA.
 */
struct AcceleratedQueryNode {
  const std::vector<StreamDataParameters> input_streams;
  const std::vector<StreamDataParameters> output_streams;
  const operation_types::QueryOperationType operation_type;
  const int operation_module_location;
  const std::vector<std::vector<int>> operation_parameters;

  //auto operator==(const AcceleratedQueryNode& rhs) const -> bool {
  //  return input_streams == rhs.input_streams &&
  //         output_streams == rhs.output_streams &&
  //         operation_type == rhs.operation_type &&
  //         operation_parameters == rhs.operation_parameters;
  //}
};

}  // namespace fpga_managing
}  // namespace dbmstodspi