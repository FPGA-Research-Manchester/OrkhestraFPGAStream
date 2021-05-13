#pragma once

namespace dbmstodspi::fpga_managing::modules {

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

}  // namespace dbmstodspi::fpga_managing::modules