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

#include "logger.hpp"

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

using orkhestrafs::dbmstodspi::DMA;

void DMA::SetControllerParams(bool is_input, int stream_id, int ddr_burst_size,
                              int records_per_ddr_burst, int buffer_start,
                              int buffer_end) {
  Log(LogLevel::kTrace,
      "Controller params: is_input - " +
          std::to_string(static_cast<int>(is_input)) + "; stream_id - " +
          std::to_string(stream_id) + "; ddr_burst_size - " +
          std::to_string(ddr_burst_size) + "; records_per_ddr_burst - " +
          std::to_string(records_per_ddr_burst) + "; buffer_start - " +
          std::to_string(buffer_start) + "; buffer_end - " +
          std::to_string(buffer_end));
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
  Log(LogLevel::kTrace, "Address: is_input - " +
                            std::to_string(static_cast<int>(is_input)) +
                            "; stream_id - " + std::to_string(stream_id) +
                            "; address - " + std::to_string(address));
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
  Log(LogLevel::kTrace, "Stream size: is_input - " +
                            std::to_string(static_cast<int>(is_input)) +
                            "; stream_id - " + std::to_string(stream_id) +
                            "; size - " + std::to_string(size));
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
  Log(LogLevel::kTrace, "Stream record size: stream_id - " +
                            std::to_string(stream_id) + "; record_size - " +
                            std::to_string(record_size));
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
  Log(LogLevel::kTrace,
      "Mul channels stream count: number - " + std::to_string(number));
  AccelerationModule::WriteToModule(4, number);
}
void DMA::SetRecordsPerBurstForMultiChannelStreams(
    int stream_id, int records_per_burst) {  // possible values 1-32
  Log(LogLevel::kTrace, "Record per burst for mul channels: stream_id - " +
                            std::to_string(stream_id) +
                            "; records_per_burst - " +
                            std::to_string(records_per_burst));
  AccelerationModule::WriteToModule(0x80000 + (stream_id * 4),
                                    records_per_burst);
}
void DMA::SetDDRBurstSizeForMultiChannelStreams(int stream_id,
                                                int ddr_burst_size) {
  Log(LogLevel::kTrace, "DDRBurst for multi channel: stream_id - " +
                            std::to_string(stream_id) + "; ddr_burst_size - " +
                            std::to_string(ddr_burst_size));
  AccelerationModule::WriteToModule(0x80000 + (1 << 6) + (stream_id * 4),
                                    ddr_burst_size - 1);
}
void DMA::SetNumberOfActiveChannelsForMultiChannelStreams(
    int stream_id,
    int active_channels) {  // possible values 1 to the synthesized channel
                            // capacity (1024 currently)
  Log(LogLevel::kTrace, "Number of channels for mul channel: stream_id - " +
                            std::to_string(stream_id) + "; active_channels - " +
                            std::to_string(active_channels));
  AccelerationModule::WriteToModule(0x80000 + (2 << 6) + (stream_id * 4),
                                    active_channels);
}
void DMA::SetAddressForMultiChannelStreams(int stream_id, int channel_id,
                                           uintptr_t address) {
  Log(LogLevel::kTrace, "Address for mul channel: stream_id - " +
                            std::to_string(stream_id) + "; channel_id - " +
                            std::to_string(channel_id) + "; address - " +
                            std::to_string(address));
  AccelerationModule::WriteToModule(
      0x80000 + (1 << 16) + (stream_id << 14) + (channel_id << 2),
      address >> 4);
}
void DMA::SetSizeForMultiChannelStreams(int stream_id, int channel_id,
                                        int number_of_records) {
  Log(LogLevel::kTrace,
      "Size for mul channel: stream_id - " + std::to_string(stream_id) +
          "; channel_id - " + std::to_string(channel_id) +
          "; number_of_records - " + std::to_string(number_of_records));
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

auto DMA::GetInputActiveDataCycles() -> volatile uint32_t {
  // auto thing1 = AccelerationModule::ReadFromModule(0x8020);
  // auto thing2 = AccelerationModule::ReadFromModule(0x8024);
  // auto thing3 = AccelerationModule::ReadFromModule(0x8028);
  // auto thing4 = AccelerationModule::ReadFromModule(0x802C);
  // auto thing5 = AccelerationModule::ReadFromModule(0x8030);
  // auto thing6 = AccelerationModule::ReadFromModule(0x8040);
  // auto thing7 = AccelerationModule::ReadFromModule(0x8044);
  // auto thing8 = AccelerationModule::ReadFromModule(0x8048);
  // auto thing9 = AccelerationModule::ReadFromModule(0x804C);
  // auto thing10 = AccelerationModule::ReadFromModule(0x8050);
  // auto thing11 = AccelerationModule::ReadFromModule(0x8060);
  // auto thing12 = AccelerationModule::ReadFromModule(0x8070);

  // AccelerationModule::WriteToModule(0x8020,100);
  // AccelerationModule::WriteToModule(0x8024, 100);
  // AccelerationModule::WriteToModule(0x8028, 100);
  // AccelerationModule::WriteToModule(0x802C, 100);
  // AccelerationModule::WriteToModule(0x8030, 100);
  // AccelerationModule::WriteToModule(0x8040, 100);
  // AccelerationModule::WriteToModule(0x8044, 100);
  // AccelerationModule::WriteToModule(0x8048, 100);
  // AccelerationModule::WriteToModule(0x804C, 100);
  // AccelerationModule::WriteToModule(0x8050, 100);
  // AccelerationModule::WriteToModule(0x8060, 100);
  // AccelerationModule::WriteToModule(0x8070, 100);
  //
  // thing1 = AccelerationModule::ReadFromModule(0x8020);
  // thing2 = AccelerationModule::ReadFromModule(0x8024);
  // thing3 = AccelerationModule::ReadFromModule(0x8028);
  // thing4 = AccelerationModule::ReadFromModule(0x802C);
  // thing5 = AccelerationModule::ReadFromModule(0x8030);
  // thing6 = AccelerationModule::ReadFromModule(0x8040);
  // thing7 = AccelerationModule::ReadFromModule(0x8044);
  // thing8 = AccelerationModule::ReadFromModule(0x8048);
  // thing9 = AccelerationModule::ReadFromModule(0x804C);
  // thing10 = AccelerationModule::ReadFromModule(0x8050);
  // thing11 = AccelerationModule::ReadFromModule(0x8060);
  // thing12 = AccelerationModule::ReadFromModule(0x8070);

  //// 0x8040: 2465647203 -> 14577 -> 3275260603
  //// 0x8044: 2465647241 -> 12233 -> 3275258259
  //// 0x8048: 2465647281 -> 9885 -> 3275255913
  //// 0x804C: 2465647319 -> 7563 -> 3275253591
  //// 0x8070: 2465647430 -> 543 -> 3275246571

  ////0x8040: X -> 14589
  ////0x8044: X -> 12259
  ////0x8048: X -> 9907
  ////0x804C: X -> 7551
  ////0x8070: X -> 541

  // thing1 = AccelerationModule::ReadFromModule(0x8020);
  // thing2 = AccelerationModule::ReadFromModule(0x8024);
  // thing3 = AccelerationModule::ReadFromModule(0x8028);
  // thing4 = AccelerationModule::ReadFromModule(0x802C);
  // thing5 = AccelerationModule::ReadFromModule(0x8030);
  // thing6 = AccelerationModule::ReadFromModule(0x8040);
  // thing7 = AccelerationModule::ReadFromModule(0x8044);
  // thing8 = AccelerationModule::ReadFromModule(0x8048);
  // thing9 = AccelerationModule::ReadFromModule(0x804C);
  // thing10 = AccelerationModule::ReadFromModule(0x8050);
  // thing11 = AccelerationModule::ReadFromModule(0x8060);
  // thing12 = AccelerationModule::ReadFromModule(0x8070);

  return AccelerationModule::ReadFromModule(0x8020);
}
auto DMA::GetInputActiveDataLastCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8024);
}
auto DMA::GetInputActiveControlCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8028);
}
auto DMA::GetInputActiveControlLastCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x802C);
}
auto DMA::GetInputActiveEndOfStreamCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8030);
}
auto DMA::GetOutputActiveDataCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8040);
}
auto DMA::GetOutputActiveDataLastCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8044);
}
auto DMA::GetOutputActiveControlCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8048);
}
auto DMA::GetOutputActiveControlLastCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x804C);
}
auto DMA::GetOutputActiveEndOfStreamCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8050);
}
auto DMA::GetInputActiveInstructionCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8060);
}
auto DMA::GetOutputActiveInstructionCycles() -> volatile uint32_t {
  return AccelerationModule::ReadFromModule(0x8070);
}

void DMA::GlobalReset() {
  AccelerationModule::WriteToModule(8, kResetDuration_);
}

void DMA::DecoupleFromPRRegion() { AccelerationModule::WriteToModule(12, 1); }
