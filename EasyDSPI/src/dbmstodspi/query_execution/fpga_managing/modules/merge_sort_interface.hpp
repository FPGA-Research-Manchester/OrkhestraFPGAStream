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

#pragma once

namespace easydspi::dbmstodspi {

/**
 * @brief Interface class for the merge sorting implemented by #MergeSort
 */
class MergeSortInterface {
 public:
  virtual ~MergeSortInterface() = default;

  virtual void StartPrefetchingData(int base_channel_id,
                                    bool is_not_first_module) = 0;
  virtual void SetStreamParams(int stream_id, int chunks_per_record) = 0;
  virtual void SetBufferSize(int record_count) = 0;
  virtual void SetRecordCountPerFetch(int record_count) = 0;
  virtual void SetFetchCount(int fetch_count) = 0;
  virtual void SetFetchOffset(int offset_record_count) = 0;
};

}  // namespace easydspi::dbmstodspi