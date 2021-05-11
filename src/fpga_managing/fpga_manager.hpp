#pragma once
#include <array>
#include <bitset>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "accelerated_query_node.hpp"
#include "aggregation_sum.hpp"
#include "dma.hpp"
#include "filter.hpp"
#include "ila.hpp"
#include "memory_manager_interface.hpp"
#include "read_back_module_interface.hpp"
#include "stream_data_parameters.hpp"

namespace dbmstodspi {
namespace fpga_managing {

/**
 * @brief Class to configure all of the modules loaded in with the bitstream.
 * Then start the streaming to accelerate the query node.
 */
class FPGAManager {
 public:
  /**
   * @brief Reset and setup all of the loaded modules and the DMA engine with
   * the input and output crossbars.
   * @param query_nodes Which query nodes will be run with the current
   * bitstream.
   */
  void SetupQueryAcceleration(
      const std::vector<AcceleratedQueryNode>& query_nodes);
  /**
   * @brief Start the output controller and wait for the controllers to finish.
   * @return How many records each output stream had in its results.
   */
  auto RunQueryAcceleration()
      -> std::array<int, query_acceleration_constants::kMaxIOStreamCount>;

  /**
   * @brief Constructor to setup memory mapped registers.
   * @param memory_manager Instance to access memory mapped registers.
   */
  explicit FPGAManager(MemoryManagerInterface* memory_manager)
      : memory_manager_{memory_manager}, dma_engine_{memory_manager} {};

 private:
  // Could be array<bool> as well since none of the bitset functions are being used.
  std::bitset<query_acceleration_constants::kMaxIOStreamCount>
      input_streams_active_status_;
  std::bitset<query_acceleration_constants::kMaxIOStreamCount>
      output_streams_active_status_;

  MemoryManagerInterface* memory_manager_;
  modules::DMA dma_engine_;
  std::optional<ILA> ila_module_;
  std::vector<std::unique_ptr<modules::ReadBackModuleInterface>>
      read_back_modules_;
  std::vector<std::vector<int>> read_back_parameters_;

  void FindActiveStreams(std::vector<int>& active_input_stream_ids,
                         std::vector<int>& active_output_stream_ids);
  void WaitForStreamsToFinish();
  void ReadResultsFromRegisters();
  auto GetResultingStreamSizes(const std::vector<int>& active_input_stream_ids,
                               const std::vector<int>& active_output_stream_ids)
      -> std::array<int, query_acceleration_constants::kMaxIOStreamCount>;
  void PrintDebuggingData();
  static void FindIOStreams(
      const std::vector<StreamDataParameters>& all_streams,
      std::vector<StreamDataParameters>& found_streams,
      const std::vector<std::vector<int>>& operation_parameters,
      bool is_multichannel_stream,
      std::bitset<query_acceleration_constants::kMaxIOStreamCount>&
          stream_status_array);

  static auto GetStreamRecordSize(const StreamDataParameters& stream_parameters)
      -> int;

  static auto ReadModuleResultRegisters(
      std::unique_ptr<modules::ReadBackModuleInterface> read_back_module,
      int position) -> double;
};

}  // namespace fpga_managing
}  // namespace dbmstodspi