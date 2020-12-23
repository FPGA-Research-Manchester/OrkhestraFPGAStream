#pragma once
#include "acceleration_module.hpp"
#include "merge_sort_interface.hpp"
#include "memory_manager_interface.hpp"
class MergeSort: public AccelerationModule, public MergeSortInterface
{
 public:
  ~MergeSort() = default;
  explicit MergeSort(MemoryManagerInterface* memory_manager,
                     int module_position)
      : AccelerationModule(memory_manager, module_position){};

  void StartPrefetchingData(int base_channel_id, bool is_not_first_module) override;
  void SetStreamParams(int stream_id, int chunks_per_record) override;
  void SetBufferSize(int record_count) override;
  void SetRecordCountPerFetch(int record_count) override;
  void SetFetchCount(int fetch_count) override;
  void SetFetchOffset(int padded_fetch_count) override;
};
