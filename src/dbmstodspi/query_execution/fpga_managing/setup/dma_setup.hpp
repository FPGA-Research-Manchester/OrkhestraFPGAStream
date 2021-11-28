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
#include <cstdint>
#include <vector>

#include "dma.hpp"
#include "dma_setup_data.hpp"
#include "dma_setup_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to setup DMA configuration data and write the data into the DMA
 * registers.
 */
class DMASetup : public DMASetupInterface {
 public:
  /**
   * @brief Method to setup both input and output streams.
   * @param dma_module Module to access registers.
   * @param input_streams Input stream parameters.
   * @param output_streams Output stream parameters.
   */
  void SetupDMAModule(
      DMAInterface &dma_module,
      const std::vector<StreamDataParameters> &input_streams,
      const std::vector<StreamDataParameters> &output_streams) override;
  /**
   * @brief Method to create a DMA module
   * @param memory_manager Manager to access register space
   * @return Smart pointer to the DMA module.
   */
  auto CreateDMAModule(MemoryManagerInterface *memory_manager)
      -> std::unique_ptr<DMAInterface> override;

 private:
  /**
   * @brief Based on the input stream parameters calculate DMA setup data and
   * then write the configuration data to the memory mapped registers.
   * @param dma_engine The modules where the information about memory mapped
   * registers is.
   * @param streams Vector of definition data for all of the input or output
   * streams.
   * @param is_input_stream Boolean saying if the given streams are input or
   * output streams.
   */
  static void SetupDMAModuleDirection(
      DMAInterface &dma_engine,
      const std::vector<StreamDataParameters> &streams, bool is_input_stream);
  static auto CalculateMultiChannelStreamRecordCountPerChannel(
      int stream_record_count, int max_channel_count, int record_size) -> int;
  static void SetUpDMAIOStream(const DMASetupData &stream_setup_data,
                               DMAInterface &dma_engine);
  static void SetUpDMACrossbarsForStream(const DMASetupData &stream_setup_data,
                                         DMAInterface &dma_engine);
  static void SetSingleChannelSetupData(
      DMASetupData &stream_setup_data, const bool &is_input_stream,
      const StreamDataParameters &stream_init_data);
  static void SetMultiChannelSetupData(
      const StreamDataParameters &stream_init_data,
      DMASetupData &stream_setup_data);
  static void AllocateStreamBuffers(DMASetupData &stream_setup_data,
                                    const int &buffer_size,
                                    int current_stream_count);
};

}  // namespace orkhestrafs::dbmstodspi