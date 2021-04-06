#pragma once
#include <cstdint>
#include <vector>
/**
 * Struct for holding data about how to configure a stream in the DMA engine.
 */
struct StreamDataParameters {
  /// ID
  const int stream_id;
  /// How many integers worth of data does a record hold.
  const int stream_record_size;
  /// How many records there are in this stream.
  const int stream_record_count;
  /// Pointer to the start of the data.
  const volatile uint32_t* physical_address;
  /// How the data should be ordered.
  const std::vector<int> stream_specification;
  /// How many chunks are used for each record before the output crossbar.
  const int input_chunks_per_record = -1;
  // For multi-channel streams
  /// How many channels can be used for this stream.
  const int max_channel_count = -1;
  /// How many records will there be for each channel.
  const int records_per_channel = -1;

  auto operator<(const StreamDataParameters& comparable) const -> bool {
    return stream_id < comparable.stream_id;
  }
};