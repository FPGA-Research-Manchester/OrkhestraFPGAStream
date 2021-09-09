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
#include <cstdio>
#include <tuple>
#include <vector>

#include "dma_crossbar_setup_data.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Struct to hold channel setup data for an individual stream.
 */
struct DMAChannelSetupData {
  /// Pointer to the start of the data.
  uintptr_t stream_address;
  /// How many records there are in this channel.
  int record_count;
  /// ID of this channel.
  int channel_id;
};

/**
 * @brief Struct to hold all configuration data required for a stream to be
 * streamed through the DMA engine.
 */
struct DMASetupData {
  int stream_id, ddr_burst_length, records_per_ddr_burst, chunks_per_record,
      buffer_start, buffer_end;
  std::vector<std::tuple<int, int>> record_chunk_ids;
  std::vector<DMACrossbarSetupData> crossbar_setup_data;
  /// If this setup data is for an input or output stream.
  bool is_input_stream;

  /// If -1 then this is a single channel stream.
  int active_channel_count;
  /// For single channel streams the channel setup data is still used.
  std::vector<DMAChannelSetupData> channel_setup_data;
};

}  // namespace orkhestrafs::dbmstodspi