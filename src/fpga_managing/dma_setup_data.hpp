#pragma once
#include <cstdio>
#include <tuple>
#include <vector>

#include "dma_crossbar_setup_data.hpp"

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