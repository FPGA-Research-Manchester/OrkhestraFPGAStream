#pragma once
#include <cstdio>
#include <tuple>
#include <vector>

#include "DMACrossbarSetupData.hpp"
struct DMASetupData {
  DMASetupData() : crossbar_setup_data(32) {}
  int stream_id{}, ddr_burst_length{}, records_per_ddr_burst{}, record_count{},
      chunks_per_record{}, buffer_start{}, buffer_end{};
  uintptr_t stream_address{};
  std::vector<std::tuple<int, int>> record_chunk_ids;
  std::vector<DMACrossbarSetupData> crossbar_setup_data;
  bool is_input_stream{};
};