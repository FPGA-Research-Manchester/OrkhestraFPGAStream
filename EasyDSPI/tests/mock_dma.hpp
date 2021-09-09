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

#include "dma_interface.hpp"
#include "gmock/gmock.h"

using easydspi::dbmstodspi::DMAInterface;
using easydspi::dbmstodspi::module_config_values::DMACrossbarDirectionSelection;
using easydspi::dbmstodspi::query_acceleration_constants::kMaxIOStreamCount;

class MockDMA : public DMAInterface {
  using crossbar_value_array = std::array<int, 4>;

 public:
  MOCK_METHOD(void, SetControllerParams,
              (bool is_input, int stream_id, int ddr_burst_size,
               int records_per_ddr_burst, int buffer_start, int buffer_end),
              (override));
  MOCK_METHOD(volatile uint32_t, GetControllerParams,
              (bool is_input, int stream_id), (override));
  MOCK_METHOD(void, SetControllerStreamAddress,
              (bool is_input, int stream_id, uintptr_t address), (override));
  MOCK_METHOD(volatile uintptr_t, GetControllerStreamAddress,
              (bool is_input, int stream_id), (override));
  MOCK_METHOD(void, SetControllerStreamSize,
              (bool is_input, int stream_id, int size), (override));
  MOCK_METHOD(volatile int, GetControllerStreamSize,
              (bool is_input, int stream_id), (override));
  MOCK_METHOD(void, StartController,
              (bool is_input, std::bitset<kMaxIOStreamCount> stream_active),
              (override));
  MOCK_METHOD(bool, IsControllerFinished, (bool is_input), (override));

  MOCK_METHOD(void, SetRecordSize, (int stream_id, int recordSize), (override));
  MOCK_METHOD(void, SetRecordChunkIDs,
              (int stream_id, int interfaceCycle, int chunkID), (override));

  MOCK_METHOD(void, SetCrossbarValues,
              (DMACrossbarDirectionSelection crossbar_selection, int stream_id,
               int clock_cycle, int offset,
               crossbar_value_array configuration_values),
              (override));

  MOCK_METHOD(void, SetNumberOfInputStreamsWithMultipleChannels, (int number),
              (override));
  MOCK_METHOD(void, SetRecordsPerBurstForMultiChannelStreams,
              (int stream_id, int records_per_burst), (override));
  MOCK_METHOD(void, SetDDRBurstSizeForMultiChannelStreams,
              (int stream_id, int ddr_burst_size), (override));
  MOCK_METHOD(void, SetNumberOfActiveChannelsForMultiChannelStreams,
              (int stream_id, int active_channels), (override));
  MOCK_METHOD(void, SetAddressForMultiChannelStreams,
              (int stream_id, int channel_id, uintptr_t address), (override));
  MOCK_METHOD(void, SetSizeForMultiChannelStreams,
              (int stream_id, int channel_id, int number_of_records),
              (override));

  MOCK_METHOD(volatile uint64_t, GetRuntime, (), (override));
  MOCK_METHOD(volatile uint64_t, GetValidReadCyclesCount, (), (override));
  MOCK_METHOD(volatile uint64_t, GetValidWriteCyclesCount, (), (override));

  MOCK_METHOD(void, GlobalReset, (), (override));
};