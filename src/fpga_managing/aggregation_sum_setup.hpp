#pragma once
#include "aggregation_sum_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Class to calculate the aggregating global sum acceleration module setup.
 */
class AggregationSumSetup {
 private:
 public:
  static void SetupAggregationSum(
      modules::AggregationSumInterface& aggregation_module, int stream_id);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi