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

#include "dma.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

using orkhestrafs::dbmstodspi::DMA;

void DMA::SetControllerParams(bool is_input, int stream_id, int ddr_burst_size,
                              int records_per_ddr_burst, int buffer_start,
                              int buffer_end) {
  int base_address = (is_input) ? (1 << 6) : (1 << 16);
  AccelerationModule::WriteToModule(
      base_address + stream_id * 4,
      ((ddr_burst_size - 1) << 24) +
          (static_cast<int>(log2(records_per_ddr_burst)) << 16) +
          (buffer_start << 8) + (buffer_end));
}

auto DMA::GetControllerParams(bool is_input, int stream_id)
    -> volatile uint32_t {
  int base_address = (is_input) ? (1 << 6) : (1 << 16);
  return AccelerationModule::ReadFromModule(base_address + stream_id * 4);
}

void DMA::SetControllerStreamAddress(bool is_input, int stream_id,
                                     uintptr_t address) {
  int base_address = (is_input) ? (2 << 6) : ((1 << 16) + (1 << 6));
  AccelerationModule::WriteToModule(base_address + stream_id * 4, address >> 4);
}

auto DMA::GetControllerStreamAddress(bool is_input, int stream_id)
    -> volatile uintptr_t {
  int base_address = (is_input) ? (2 << 6) : ((1 << 16) + (1 << 6));
  return AccelerationModule::ReadFromModule(base_address + stream_id * 4) << 4;
}

void DMA::SetControllerStreamSize(
    bool is_input, int stream_id,
    int size) {  // starting size of stream in amount of records
  int base_address = (is_input) ? (3 << 6) : ((1 << 16) + (2 << 6));
  AccelerationModule::WriteToModule(base_address + stream_id * 4, size);
}

auto DMA::GetControllerStreamSize(bool is_input, int stream_id)
    -> volatile int {
  int base_address = (is_input) ? (3 << 6) : ((1 << 16) + (2 << 6));
  return AccelerationModule::ReadFromModule(base_address + stream_id * 4);
}

void DMA::StartController(
    bool is_input,
    std::bitset<query_acceleration_constants::kMaxIOStreamCount>
        stream_active) {  // indicate which streams can be read from DDR
                          // and start processing
  int base_address = (is_input) ? (0) : ((1 << 16) + (3 << 6));

  int active_streams = 0;
  for (int i = query_acceleration_constants::kMaxIOStreamCount - 1; i >= 0;
       i--) {
    active_streams = active_streams << 1;
    if (stream_active[i]) {
      active_streams = active_streams + 1;
    }
  }
  AccelerationModule::WriteToModule(base_address, active_streams);
}

auto DMA::IsControllerFinished(bool is_input)
    -> bool {  // true if all streams were read from DDR
  int base_address =
      (is_input)
          ? (0)
          : ((1 << 16) +
             (3 << 6));  // input (3 << 6) && other output address in specs?
  return (AccelerationModule::ReadFromModule(base_address) == 0);
}

// How many chunks is a record on a particular stream_id
void DMA::SetRecordSize(int stream_id, int record_size) {
  AccelerationModule::WriteToModule(((1 << 17) + (1 << 8) + (stream_id * 4)),
                                    record_size - 1);
}
// set ChunkID at clock cycle of interfaceCycle for records on a particular
// stream_id
void DMA::SetRecordChunkIDs(int stream_id, int interface_cycle, int chunk_id) {
  AccelerationModule::WriteToModule(
      ((1 << 17) + (1 << 13) + (stream_id << 8) + (interface_cycle << 2)),
      chunk_id);
}

void DMA::SetCrossbarValues(
    module_config_values::DMACrossbarDirectionSelection crossbar_selection,
    int stream_id, int clock_cycle, int offset,
    std::array<int, 4> configuration_values) {
  int base_address = 0;
  switch (crossbar_selection) {
    case module_config_values::DMACrossbarDirectionSelection::
        kBufferToInterfaceChunk:
      base_address = (2 << 17) + (1 << 16);
      break;
    case module_config_values::DMACrossbarDirectionSelection::
        kBufferToInterfacePosition:
      base_address = (2 << 17);
      break;
    case module_config_values::DMACrossbarDirectionSelection::
        kInterfaceToBufferChunk:
      base_address = (3 << 17) + (1 << 16);
      break;
    case module_config_values::DMACrossbarDirectionSelection::
        kInterfaceToBufferPosition:
      base_address = (3 << 17);
      break;
    default:
      throw std::runtime_error("Incorrect input!");
      break;
  }
  AccelerationModule::WriteToModule(
      (base_address + (stream_id << 12) + (clock_cycle << 5) + (offset << 2)),
      ((configuration_values[0] << 24) + (configuration_values[1] << 16) +
       (configuration_values[2] << 8) + configuration_values[3]));
}

// MultiChannel
void DMA::SetNumberOfInputStreamsWithMultipleChannels(
    int number) {  // Number of special channeled streams (for example for merge
                   // sorting) These streams would be located at StreamIDs
                   // 0..(number-1)
  AccelerationModule::WriteToModule(4, number);
}
void DMA::SetRecordsPerBurstForMultiChannelStreams(
    int stream_id, int records_per_burst) {  // possible values 1-32
  AccelerationModule::WriteToModule(0x80000 + (stream_id * 4),
                                    records_per_burst);
}
void DMA::SetDDRBurstSizeForMultiChannelStreams(int stream_id,
                                                int ddr_burst_size) {
  AccelerationModule::WriteToModule(0x80000 + (1 << 6) + (stream_id * 4),
                                    ddr_burst_size - 1);
}
void DMA::SetNumberOfActiveChannelsForMultiChannelStreams(
    int stream_id,
    int active_channels) {  // possible values 1 to the synthesized channel
                            // capacity (1024 currently)
  AccelerationModule::WriteToModule(0x80000 + (2 << 6) + (stream_id * 4),
                                    active_channels);
}
void DMA::SetAddressForMultiChannelStreams(int stream_id, int channel_id,
                                           uintptr_t address) {
  AccelerationModule::WriteToModule(
      0x80000 + (1 << 16) + (stream_id << 14) + (channel_id << 2),
      address >> 4);
}
void DMA::SetSizeForMultiChannelStreams(int stream_id, int channel_id,
                                        int number_of_records) {
  AccelerationModule::WriteToModule(
      0x80000 + (2 << 16) + (stream_id << 14) + (channel_id << 2),
      number_of_records + 1);
}

// Debugging
auto DMA::GetRuntime() -> volatile uint64_t {
  auto high = AccelerationModule::ReadFromModule(0x8000);
  auto low = AccelerationModule::ReadFromModule(0x8004);
  return ((static_cast<uint64_t>(high)) << 32) | (static_cast<uint64_t>(low));
}
auto DMA::GetValidReadCyclesCount() -> volatile uint64_t {
  auto high = AccelerationModule::ReadFromModule(0x8008);
  auto low = AccelerationModule::ReadFromModule(0x800c);
  return ((static_cast<uint64_t>(high)) << 32) | (static_cast<uint64_t>(low));
}
auto DMA::GetValidWriteCyclesCount() -> volatile uint64_t {
  auto high = AccelerationModule::ReadFromModule(0x8010);
  auto low = AccelerationModule::ReadFromModule(0x8014);
  return ((static_cast<uint64_t>(high)) << 32) | (static_cast<uint64_t>(low));
}

void DMA::GlobalReset() {
  AccelerationModule::WriteToModule(8, kResetDuration_);
}

void DMA::DecoupleFromPRRegion() { AccelerationModule::WriteToModule(12, 1); }
