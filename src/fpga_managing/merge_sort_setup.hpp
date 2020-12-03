#pragma once
#include "merge_sort_interface.hpp"
class MergeSortSetup {
 public:
  static void SetupMergeSortModule(MergeSortInterface& merge_sort_module,
                                   int stream_id,
                                   int chunks_per_record, int base_channel_id,
                                   bool is_first);

 private:
  static int CalculateSortBufferSize(int buffer_space,
                                     int channel_count,
                                     int chunks_per_record);
};