#include "merge_sort.hpp"

using namespace dbmstodspi::fpga_managing::modules;

void MergeSort::StartPrefetchingData(int base_channel_id,
                                     bool is_not_first_module) {
  AccelerationModule::WriteToModule(0,
                                    (base_channel_id << 8) + static_cast<int>(is_not_first_module));
}

void MergeSort::SetStreamParams(int stream_id, int chunks_per_record) {
  AccelerationModule::WriteToModule(4, ((chunks_per_record - 1) << 8) + stream_id);
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

void MergeSort::SetFetchOffset(int offset_record_count) {
  AccelerationModule::WriteToModule(20, offset_record_count);
}
