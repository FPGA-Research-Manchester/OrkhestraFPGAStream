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

/**
 * @brief Class to configure all of the modules loaded in with the bitstream.
 * Then start the streaming to accelerate the query node.
 */
class FPGAManager {
 public:
  /**
   * @brief Reset and setup all of the loaded modules and the DMA engine with
   * the input and output crossbars.
   * @param available_modules Which modules have been loaded in the current
   * bitstream.
   * @param query_nodes Which query nodes will be run with the current
   * bitstream.
   */
  void SetupQueryAcceleration(
      const std::vector<operation_types::QueryOperation>& available_modules,
      const std::vector<AcceleratedQueryNode>& query_nodes);
  /**
   * @brief Start the output controller and wait for the controllers to finish.
   * @return How many records each output stream had in its results.
   */
  auto RunQueryAcceleration() -> std::vector<int>;

  /**
   * @brief Constructor to setup memory mapped registers.
   * @param memory_manager Instance to access memory mapped registers.
   */
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
  static void FindIOStreams(
      const std::vector<StreamDataParameters>& all_streams,
      std::vector<StreamDataParameters>& found_streams,
      const std::vector<std::vector<int>>& operation_parameters,
      bool is_multichannel_stream, bool stream_status_array[]);

  static auto GetStreamRecordSize(const StreamDataParameters& stream_parameters)
      -> int;
};