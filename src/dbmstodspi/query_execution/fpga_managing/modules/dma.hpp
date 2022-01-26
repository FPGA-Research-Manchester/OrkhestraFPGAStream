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

#include "acceleration_module.hpp"
#include "dma_interface.hpp"
#include "memory_manager_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief DMA module which handles AXI bursts and sends data to the acceleration
 * modules.
 */
class DMA : public AccelerationModule, public DMAInterface {
 public:
  ~DMA() override = default;
  /**
   * @brief Constructor to setup the DMA module configuration class.
   * @param memory_manager Memory manager instance to access memory mapped
   * registers.
   */
  explicit DMA(MemoryManagerInterface* memory_manager)
      : AccelerationModule(memory_manager, 0){};

  /**
   * @brief Give stream information to the input/output controller.
   * @param is_input Bool noting if the configuration is for input or
   * output.
   * @param stream_id ID of the stream.
   * @param ddr_burst_size How many clock cycles should the DDR burst be
   * for the input.
   * @param records_per_ddr_burst How many records are going to be
   * transfered during a DDR burst.
   * @param buffer_start Index of a buffer where the stream will be
   * buffered to.
   * @param buffer_end Index of a buffer up to where the stream can be
   * buffered to.
   */
  void SetControllerParams(bool is_input, int stream_id, int ddr_burst_size,
                           int records_per_ddr_burst, int buffer_start,
                           int buffer_end) override;
  /**
   * @brief Read input/output controller parameters.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_id ID of the stream to read.
   * @return Read configuration data.
   */
  auto GetControllerParams(bool is_input, int stream_id)
      -> volatile uint32_t override;
  /**
   * @brief Set the starting address of a stream in DDR.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_id ID of the stream to be configured.
   * @param address Pointer to the starting point of the stream data.
   */
  void SetControllerStreamAddress(bool is_input, int stream_id,
                                  uintptr_t address) override;
  /**
   * @brief Read stream's starting address.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_id ID of the stream.
   * @return Pointer to the start of the stream's data in DDR.
   */
  auto GetControllerStreamAddress(bool is_input, int stream_id)
      -> volatile uintptr_t override;
  /**
   * @brief Set how many records does the input/output stream have.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_id ID of the input/output stream.
   * @param size Number of records in the input/output stream.
   */
  void SetControllerStreamSize(bool is_input, int stream_id, int size) override;
  /**
   * @brief Read how many records does the input/output stream have.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_id ID of the input/output stream.
   * @return Number of records in the input/output stream.
   */
  auto GetControllerStreamSize(bool is_input, int stream_id)
      -> volatile int override;
  /**
   * @brief Start the input/output controller to start buffering streams and
   * then sending them to the acceleration modules.
   * @param is_input Bool noting if the configuration is for input or output.
   * @param stream_active Array of booleans showing which streams should start
   * streaming towards the modules.
   */
  void StartController(
      bool is_input,
      std::bitset<query_acceleration_constants::kMaxIOStreamCount>
          stream_active) override;
  /**
   * @brief Find out if input/output controller has fully streamed all the
   * streams.
   * @param is_input Bool noting if the configuration is for input or output.
   * @return Boolean showing if the input/output controller has finished.
   */
  auto IsControllerFinished(bool is_input) -> bool override;

  /**
   * @brief Set record size in terms of integers of a stream.
   * @param stream_id ID of the configured stream.
   * @param record_size How many integers of data does a record contain.
   */
  void SetRecordSize(int stream_id, int record_size) override;
  /**
   * @brief Give chunk IDs to different chunks of a record.
   * @param stream_id ID of the configured stream.
   * @param interface_cycle Which chunk is getting configured.
   * @param chunk_id The given ID.
   */
  void SetRecordChunkIDs(int stream_id, int interface_cycle,
                         int chunk_id) override;

  /**
   * @brief Method to configure the input/output crossbar chunks/position
   * selection.
   *
   * In-depth explanation is in the README.
   * @param crossbar_selection Which crossbar is being configured.
   * @param stream_id Which input stream is being configured.
   * @param clock_cycle Which clock cycle is having its positions/chunks set.
   * @param offset Which 4 positions/chunks are being set.
   * @param configuration_values Values of 4 selections in reverse order.
   */
  void SetCrossbarValues(
      module_config_values::DMACrossbarDirectionSelection crossbar_selection,
      int stream_id, int clock_cycle, int offset,
      std::array<int, 4> configuration_values) override;

  /**
   * @brief Set the number of input streams which are going to have multiple
   * channels.
   * @param number Multichannel stream count.
   */
  void SetNumberOfInputStreamsWithMultipleChannels(int number) override;
  /**
   * @brief Set the number of records that will be streamed within a single
   * burst for multichannel streams.
   * @param stream_id Multichannel stream ID.
   * @param records_per_burst Number of records that will be streamed during a
   * DDR burst.
   */
  void SetRecordsPerBurstForMultiChannelStreams(int stream_id,
                                                int records_per_burst) override;
  /**
   * @brief Set the DDR burst length for the given multichannel stream.
   * @param stream_id Input multichannel stream ID.
   * @param ddr_burst_size How many clock cycles the DDR burst lasts.
   */
  void SetDDRBurstSizeForMultiChannelStreams(int stream_id,
                                             int ddr_burst_size) override;
  /**
   * @brief Set the number of used channels for the given multichannel stream.
   * @param stream_id Input multichannels stream ID.
   * @param active_channels How many active channels there are for this stream.
   */
  void SetNumberOfActiveChannelsForMultiChannelStreams(
      int stream_id, int active_channels) override;
  /**
   * @brief Set the starting address of a stream's channel.
   * @param stream_id Multichannel stream ID.
   * @param channel_id Channel ID.
   * @param address Pointer to the start of the stream's data in DDR.
   */
  void SetAddressForMultiChannelStreams(int stream_id, int channel_id,
                                        uintptr_t address) override;
  /**
   * @brief Set how many records should be streamed into the given channel of
   * the given stream.
   * @param stream_id Multichannel stream ID.
   * @param channel_id Stream's channel ID.
   * @param number_of_records Number of records to be streamed into the given
   * channel.
   */
  void SetSizeForMultiChannelStreams(int stream_id, int channel_id,
                                     int number_of_records) override;

  /**
   * @brief Debug method to get the runtime of the run. Measured by the
   * hardware.
   * @return Amount of clock cycles spent on the execution of the query node.
   */
  auto GetRuntime() -> volatile uint64_t override;
  /**
   * @brief Debug method to read the valid read cycle counter register.
   * @return Number of cycles there were valid reads.
   */
  auto GetValidReadCyclesCount() -> volatile uint64_t override;
  /**
   * @brief Debug method to read the valid write cycle counter register.
   * @return Number of cycles there were valid writes.
   */
  auto GetValidWriteCyclesCount() -> volatile uint64_t override;

  ///Debug methods
  auto GetInputActiveDataCycles() -> volatile uint32_t override;
  auto GetInputActiveDataLastCycles() -> volatile uint32_t override;
  auto GetInputActiveControlCycles() -> volatile uint32_t override;
  auto GetInputActiveControlLastCycles() -> volatile uint32_t override;
  auto GetInputActiveEndOfStreamCycles() -> volatile uint32_t override;
  auto GetOutputActiveDataCycles() -> volatile uint32_t override;
  auto GetOutputActiveDataLastCycles() -> volatile uint32_t override;
  auto GetOutputActiveControlCycles() -> volatile uint32_t override;
  auto GetOutputActiveControlLastCycles() -> volatile uint32_t override;
  auto GetOutputActiveEndOfStreamCycles() -> volatile uint32_t override;
  auto GetInputActiveInstructionCycles() -> volatile uint32_t override;
  auto GetOutputActiveInstructionCycles() -> volatile uint32_t override;

  /**
   * @brief Method to write to the reset register to reset all configured
   * modules including the DMA.
   *
   * The reset signal will be high for 8 clock cycles.
   */
  void GlobalReset() override;

  /**
   * @brief Method to decouple the DMA from the PR region for partial reconfiguration.
   * 
   * To renable the connection global reset has to be used.
   */
  void DecoupleFromPRRegion() override;

  const int kResetDuration_ = 8;
};

}  // namespace orkhestrafs::dbmstodspi
