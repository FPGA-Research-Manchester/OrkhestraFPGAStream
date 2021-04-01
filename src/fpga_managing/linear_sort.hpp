#pragma once
#include "acceleration_module.hpp"
#include "linear_sort_interface.hpp"
#include "memory_manager_interface.hpp"

class LinearSort : public AccelerationModule, public LinearSortInterface {
 private:
 public:
  ~LinearSort() override = default;
  explicit LinearSort(MemoryManagerInterface* memory_manager,
                      int module_position)
      : AccelerationModule(memory_manager, module_position){};

  void SetStreamParams(int stream_id, int chunks_per_record) override;
  void StartPrefetchingData() override;
};