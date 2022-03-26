/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "merge_sort.hpp"

#include <string>

#include "logger.hpp"

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

using orkhestrafs::dbmstodspi::MergeSort;

void MergeSort::StartPrefetchingData(int base_channel_id,
                                     bool is_not_first_module) {
  Log(LogLevel::kTrace,
      "Start prefetch: channel - " + std::to_string(base_channel_id) +
          "; not first - " +
          std::to_string(static_cast<int>(is_not_first_module)));
  AccelerationModule::WriteToModule(
      0, (base_channel_id << 8) + static_cast<int>(is_not_first_module));
}

void MergeSort::SetStreamParams(int stream_id, int chunks_per_record) {
  Log(LogLevel::kTrace, "Params: stream_id - " + std::to_string(stream_id) +
                            "; chunks_per_record - " +
                            std::to_string(chunks_per_record));
  AccelerationModule::WriteToModule(4,
                                    ((chunks_per_record - 1) << 8) + stream_id);
}

void MergeSort::SetBufferSize(int record_count) {
  Log(LogLevel::kTrace,
      "Buffer size: record_count - " + std::to_string(record_count));
  AccelerationModule::WriteToModule(8, record_count);
}

void MergeSort::SetRecordCountPerFetch(int record_count) {
  Log(LogLevel::kTrace,
      "Record count per fetch: record_count - " + std::to_string(record_count));
  AccelerationModule::WriteToModule(12, record_count);
}

void MergeSort::SetFetchCount(int fetch_count) {
  Log(LogLevel::kTrace,
      "Fetch count: fetch_count - " + std::to_string(fetch_count));
  AccelerationModule::WriteToModule(16, fetch_count);
}

void MergeSort::SetFetchOffset(int offset_record_count) {
  Log(LogLevel::kTrace, "Fetch offset: offset_record_count - " +
                            std::to_string(offset_record_count));
  AccelerationModule::WriteToModule(20, offset_record_count);
}
