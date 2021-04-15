#include "aggregation_sum.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void dbmstodspi::fpga_managing::modules::AggregationSum::StartPrefetching(
    bool request_data, bool destroy_data, bool count_data) {
  AccelerationModule::WriteToModule(
      0, (request_data << 2) + (destroy_data << 1) + count_data);
}

void dbmstodspi::fpga_managing::modules::AggregationSum::DefineInput(
    int stream_id, int chunk_id) {
  AccelerationModule::WriteToModule(4, (chunk_id << 8) + stream_id);
}

auto dbmstodspi::fpga_managing::modules::AggregationSum::ReadSum(
    int data_position, bool is_low) -> uint32_t {
  return AccelerationModule::ReadFromModule(64 + data_position * 8 +
                                            (is_low * 4));
}

auto dbmstodspi::fpga_managing::modules::AggregationSum::IsModuleActive()
    -> bool {
  return 1 & AccelerationModule::ReadFromModule(4);
}

void dbmstodspi::fpga_managing::modules::AggregationSum::ResetSumRegisters() {
  AccelerationModule::WriteToModule(64, 1);
}
