#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "accelerated_query_node.hpp"
#include "dma.hpp"
#include "filter.hpp"
#include "ila.hpp"
#include "memory_manager_interface.hpp"
#include "stream_data_parameters.hpp"

class FPGAManager {
 public:
  void SetupQueryAcceleration(
      const std::vector<operation_types::QueryOperation>& available_modules,
      const std::vector<AcceleratedQueryNode>& query_nodes);
  auto RunQueryAcceleration() -> std::vector<int>;

  explicit FPGAManager(MemoryManagerInterface* memory_manager)
      : memory_manager_{memory_manager}, dma_engine_{memory_manager} {};

 private:
  const static int kMaxStreamAmount = 16;
  bool input_streams_active_status_[kMaxStreamAmount] = {false};
  bool output_streams_active_status_[kMaxStreamAmount] = {false};

  MemoryManagerInterface* memory_manager_;
  DMA dma_engine_;
  std::optional<ILA> ila_module_;

  void FindActiveStreams(std::vector<int>& active_input_stream_ids,
                         std::vector<int>& active_output_stream_ids);
  void WaitForStreamsToFinish();
  auto GetResultingStreamSizes(const std::vector<int>& active_input_stream_ids,
                               const std::vector<int>& active_output_stream_ids)
      -> std::vector<int>;
  void PrintDebuggingData();
  void FindIOStreams(
      const std::vector<StreamDataParameters> all_streams,
      std::vector<std::pair<StreamDataParameters, bool>>& found_streams,
      const bool is_multichannel_stream, bool stream_status_array[]);
};