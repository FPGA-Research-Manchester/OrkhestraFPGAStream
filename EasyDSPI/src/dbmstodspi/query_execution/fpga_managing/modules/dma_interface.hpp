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
#include <array>
#include <bitset>
#include <cstdint>

#include "module_config_values.hpp"
#include "query_acceleration_constants.hpp"

namespace easydspi::dbmstodspi {

/**
 * @brief Interface class whose implementation can be seen at #DMA.
 */
class DMAInterface {
 public:
  virtual ~DMAInterface() = default;

  virtual void SetControllerParams(bool is_input, int stream_id,
                                   int ddr_burst_size,
                                   int records_per_ddr_burst, int buffer_start,
                                   int buffer_end) = 0;
  virtual auto GetControllerParams(bool is_input, int stream_id)
      -> volatile uint32_t = 0;
  virtual void SetControllerStreamAddress(bool is_input, int stream_id,
                                          uintptr_t address) = 0;
  virtual auto GetControllerStreamAddress(bool is_input, int stream_id)
      -> volatile uintptr_t = 0;
  virtual void SetControllerStreamSize(bool is_input, int stream_id,
                                       int size) = 0;
  virtual auto GetControllerStreamSize(bool is_input, int stream_id)
      -> volatile int = 0;
  virtual void StartController(
      bool is_input,
      std::bitset<query_acceleration_constants::kMaxIOStreamCount>
          stream_active) = 0;
  virtual auto IsControllerFinished(bool is_input) -> bool = 0;

  virtual void SetRecordSize(int stream_id, int record_size) = 0;
  virtual void SetRecordChunkIDs(int stream_id, int interface_cycle,
                                 int chunk_id) = 0;

  virtual void SetCrossbarValues(
      module_config_values::DMACrossbarDirectionSelection crossbar_selection,
      int stream_id, int clock_cycle, int offset,
      std::array<int, 4> configuration_values) = 0;

  virtual void SetNumberOfInputStreamsWithMultipleChannels(int number) = 0;
  virtual void SetRecordsPerBurstForMultiChannelStreams(
      int stream_id, int records_per_burst) = 0;
  virtual void SetDDRBurstSizeForMultiChannelStreams(int stream_id,
                                                     int ddr_burst_size) = 0;
  virtual void SetNumberOfActiveChannelsForMultiChannelStreams(
      int stream_id, int active_channels) = 0;
  virtual void SetAddressForMultiChannelStreams(int stream_id, int channel_id,
                                                uintptr_t address) = 0;
  virtual void SetSizeForMultiChannelStreams(int stream_id, int channel_id,
                                             int number_of_records) = 0;

  virtual auto GetRuntime() -> volatile uint64_t = 0;
  virtual auto GetValidReadCyclesCount() -> volatile uint64_t = 0;
  virtual auto GetValidWriteCyclesCount() -> volatile uint64_t = 0;

  virtual void GlobalReset() = 0;
};

}  // namespace easydspi::dbmstodspi