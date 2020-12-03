#include "merge_sort.hpp"

void MergeSort::StartPrefetchingData(int base_channel_id,
                                     bool is_first_module) {
  AccelerationModule::WriteToModule(0,
                                    (base_channel_id << 8) + static_cast<int>(is_first_module));
}

void MergeSort::SetStreamParams(int stream_id, int chunks_per_record) {
  AccelerationModule::WriteToModule(4, (chunks_per_record << 8) + stream_id);
}

void MergeSort::SetBufferSize(int record_count) {
  AccelerationModule::WriteToModule(8, record_count);
}

void MergeSort::SetRecordCountPerFetch(int record_count) {
  AccelerationModule::WriteToModule(12, record_count);
}

void MergeSort::SetFetchCount(int fetch_count) {
  AccelerationModule::WriteToModule(16, fetch_count);
}

void MergeSort::SetFetchOffset(int padded_fetch_count) {
  AccelerationModule::WriteToModule(20, padded_fetch_count);
}

void MergeSort::Reset() {
  AccelerationModule::WriteToModule(24, 1);
}
