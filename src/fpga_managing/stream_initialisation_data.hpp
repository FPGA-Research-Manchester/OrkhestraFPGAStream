#pragma once
#include <string>
#include <memory>
#include "memory_block_interface.hpp"
struct StreamInitialisationData {
  int stream_id;
  std::string stream_data_file_name;
  std::unique_ptr<MemoryBlockInterface> memory_block;
};