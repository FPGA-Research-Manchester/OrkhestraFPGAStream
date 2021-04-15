#include "aggregation_sum_setup.hpp"

using namespace dbmstodspi::fpga_managing;

void dbmstodspi::fpga_managing::AggregationSumSetup::SetupAggregationSum(
    modules::AggregationSumInterface& aggregation_module, int stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  aggregation_module.ResetSumRegisters();
  aggregation_module.DefineInput(stream_id, operation_parameters.at(0).at(0));
  aggregation_module.StartPrefetching(false, false, false);
}
