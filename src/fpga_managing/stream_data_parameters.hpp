#pragma once
#include <vector>
#include <cstdint>
struct StreamDataParameters {
  int stream_id;
  int stream_record_size;
  int stream_record_count;
  volatile uint32_t* physical_address;
};