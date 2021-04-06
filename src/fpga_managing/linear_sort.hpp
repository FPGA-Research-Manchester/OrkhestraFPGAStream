#pragma once
#include "acceleration_module.hpp"
#include "linear_sort_interface.hpp"
#include "memory_manager_interface.hpp"

/**
 * Class to implement the linear sorting operation acceleration.
 */
class LinearSort : public AccelerationModule, public LinearSortInterface {
 private:
 public:
  ~LinearSort() override = default;
  /**
   * Constructor to setup memory mapper access to configuration registers.
   * @param memory_manager Memory manager instance to access memory mapped
   *  registers.
   * @param module_position Position of this module in the bitstream.
   */
  explicit LinearSort(MemoryManagerInterface* memory_manager,
                      int module_position)
      : AccelerationModule(memory_manager, module_position){};

  /**
   * Set stream parameters such that the module knows which stream ID to look
   *  for.
   * @param stream_id Stream to be sorted.
   * @param chunks_per_record How many chunks are used for each record in the
   *  stream.
   */
  void SetStreamParams(int stream_id, int chunks_per_record) override;

  /**
   * Write to the module to start prefetching data.
   */
  void StartPrefetchingData() override;
};