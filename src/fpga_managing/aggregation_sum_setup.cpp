#include "aggregation_sum_setup.hpp"

using namespace dbmstodspi::fpga_managing;

void dbmstodspi::fpga_managing::AggregationSumSetup::SetupAggregationSum(
    modules::AggregationSumInterface& aggregation_module, int stream_id) {
  aggregation_module.ResetSumRegisters();
  aggregation_module.DefineInput(stream_id,0);
  aggregation_module.StartPrefetching(false, false, false);
}
