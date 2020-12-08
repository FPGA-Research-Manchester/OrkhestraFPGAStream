#include "merge_sort_setup.hpp"

#include "query_acceleration_constants.hpp"

void MergeSortSetup::SetupMergeSortModule(MergeSortInterface& merge_sort_module,
                                          int stream_id, int record_size,
                                          int base_channel_id, bool is_first) {
  //merge_sort_module.Reset();

  int chunks_per_record =
      (record_size + query_acceleration_constants::kDatapathWidth - 1) /
      query_acceleration_constants::kDatapathWidth;

  merge_sort_module.SetStreamParams(stream_id, chunks_per_record);

  int sort_buffer_size = CalculateSortBufferSize(2048, 64, chunks_per_record);
  int record_count_per_fetch = 2;

  merge_sort_module.SetBufferSize(sort_buffer_size);
  merge_sort_module.SetRecordCountPerFetch(record_count_per_fetch);
  merge_sort_module.SetFetchCount(sort_buffer_size / record_count_per_fetch);
  merge_sort_module.SetFetchOffset(sort_buffer_size % record_count_per_fetch);

  merge_sort_module.StartPrefetchingData(base_channel_id, !is_first);
}

auto MergeSortSetup::CalculateSortBufferSize(int buffer_space,
                                             int channel_count,
                                             int chunks_per_record) -> int {
  int internal_logic_buffer_reserve = 240;
  for (int i = 32; i <= channel_count; i *= 2) {
    internal_logic_buffer_reserve += i * 2;
  }
  int max_buffered_record_count = buffer_space / chunks_per_record -
                                  16;  // -16 for records in the pipelines.

  return (max_buffered_record_count - internal_logic_buffer_reserve) /
         channel_count;
}
