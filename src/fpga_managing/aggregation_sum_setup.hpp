#pragma once
#include <vector>

#include "aggregation_sum_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Class to calculate the aggregating global sum acceleration module
 * setup.
 */
class AggregationSumSetup {
 private:
 public:
  /**
   * @brief Setup the configuration for the global sum module.
   *
   * The operation parameters just consist of the chunk ID where the aggregation
   * should take place. The second vector is for read_back specification to know
   * which positions should get read. This data should already be reversed!
   *
   * @param aggregation_module Module instance to have access to the memory
   * mapped registers.
   * @param stream_id Which stream the aggregation operates on.
   * @param operation_parameters Data to specify how the operation should be
   * configured
   */
  static void SetupAggregationSum(
      modules::AggregationSumInterface& aggregation_module, int stream_id,
      const std::vector<std::vector<int>>& operation_parameters);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi