#pragma once
#include <cstdio>
#include <tuple>
#include <vector>

#include "dma_crossbar_setup_data.hpp"

struct DMAChannelSetupData {
  uintptr_t stream_address;
  int record_count;
  int channel_id;
};

struct DMAMultiChannelSetupData {
  int stream_id, ddr_burst_length, records_per_ddr_burst,
      chunks_per_record, buffer_start, buffer_end, active_channel_count;
  
  std::vector<std::tuple<int, int>> record_chunk_ids;
  std::vector<DMACrossbarSetupData> crossbar_setup_data;
  std::vector<DMAChannelSetupData> channel_setup_data;

  bool is_input_stream;
};