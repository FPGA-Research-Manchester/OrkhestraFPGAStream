#pragma once

#include <cstdint>

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Interface class which is implemented in the #AggregationSum class.
 */
class AggregationSumInterface {
 public:
  virtual ~AggregationSumInterface() = default;

  virtual void StartPrefetching(bool request_data, bool destroy_data,
                                bool count_data) = 0;
  virtual void DefineInput(int stream_id, int chunk_id) = 0;
  virtual auto ReadSum(int data_position, bool is_low) -> uint32_t = 0;
  virtual auto IsModuleActive() -> bool = 0;
  virtual void ResetSumRegisters() = 0;
};

}  // namespace dbmstodspi::fpga_managing::modules