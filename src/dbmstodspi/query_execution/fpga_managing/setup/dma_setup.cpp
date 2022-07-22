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

#include "dma_setup.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>

#include "dma_crossbar_setup.hpp"
#include "merge_sort_setup.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::DMASetup;

void DMASetup::SetupDMAModule(
    DMAInterface& dma_module,
    const std::vector<StreamDataParameters>& input_streams,
    const std::vector<StreamDataParameters>& output_streams) {
  /*int input_stream_size = 0;
  for (const auto& input : input_streams) {
    input_stream_size +=
        input.stream_record_count * input.stream_record_size * 4;
  }
  std::cout << "STREAMED DATA SIZE:" << input_stream_size << std::endl;*/
  DMASetup::SetupDMAModuleDirection(dma_module, input_streams, true);
  DMASetup::SetupDMAModuleDirection(dma_module, output_streams, false);
}

auto DMASetup::CreateDMAModule(MemoryManagerInterface* memory_manager)
    -> std::unique_ptr<DMAInterface> {
  return std::make_unique<DMA>(memory_manager);
}

void DMASetup::SetupDMAModuleDirection(
    DMAInterface& dma_engine, const std::vector<StreamDataParameters>& streams,
    const bool is_input_stream) {
  const int buffer_size = 16 / streams.size();
  int multichannel_stream_count = 0;
  for (int current_stream_count = 0; current_stream_count < streams.size();
       current_stream_count++) {
    const StreamDataParameters& stream_init_data =
        streams.at(current_stream_count);
    const bool is_multichannel_stream =
        (stream_init_data.max_channel_count != -1);

    DMASetupData stream_setup_data;
    stream_setup_data.is_input_stream = is_input_stream;
    stream_setup_data.stream_id = stream_init_data.stream_id;

    if (!is_multichannel_stream) {
      SetSingleChannelSetupData(stream_setup_data, is_input_stream,
                                stream_init_data);
    } else {
      multichannel_stream_count++;
      SetMultiChannelSetupData(stream_init_data, stream_setup_data);
    }

    AllocateStreamBuffers(stream_setup_data, buffer_size, current_stream_count);

    std::vector<int> stream_specification;
    if (stream_init_data.stream_specification.empty()) {
      stream_specification.resize(stream_init_data.stream_record_size);
      std::iota(stream_specification.begin(), stream_specification.end(), 0);
    } else {
      stream_specification = stream_init_data.stream_specification;
      if (!stream_setup_data.is_input_stream &&
          stream_specification.size() != stream_init_data.stream_record_size) {
        throw std::runtime_error("Expected data and projection don't match!");
      }
    }

    // For output crossbar the chunks_per_record is chosen between the interface
    // data size and the output data size to make sure that everything fits to
    // the crossbar configuration space if the projection operation does large
    // changes to the record size.
    if (!stream_setup_data.is_input_stream) {
      if (stream_init_data.input_chunks_per_record == -1) {
        throw std::runtime_error(
            "Chunk size at the end of the acceleration isn't specified!");
      }
      stream_setup_data.chunks_per_record =
          std::max(stream_init_data.input_chunks_per_record,
                   StreamParameterCalculator::CalculateChunksPerRecord(
                       stream_specification.size()));
    } else {
      stream_setup_data.chunks_per_record =
          StreamParameterCalculator::CalculateChunksPerRecord(
              stream_specification.size());
    }
    StreamParameterCalculator::CalculateDMAStreamSetupData(
        stream_setup_data, stream_init_data.stream_record_size,
        is_multichannel_stream, stream_init_data.smallest_module_size);

    SetUpDMAIOStream(stream_setup_data, dma_engine);

    DMACrossbarSetup::CalculateCrossbarSetupData(
        stream_setup_data, stream_init_data.stream_record_size,
        stream_specification);

    SetUpDMACrossbarsForStream(stream_setup_data, dma_engine);
  }

  if (is_input_stream && multichannel_stream_count != 0) {
    dma_engine.SetNumberOfInputStreamsWithMultipleChannels(
        multichannel_stream_count);
  } else if (is_input_stream) {
    dma_engine.SetNumberOfInputStreamsWithMultipleChannels(0);
  }
}

void DMASetup::AllocateStreamBuffers(DMASetupData& stream_setup_data,
                                     const int& buffer_size,
                                     int current_stream_count) {
  stream_setup_data.buffer_start = buffer_size * current_stream_count;
  stream_setup_data.buffer_end =
      stream_setup_data.buffer_start + buffer_size - 1;
}

void DMASetup::SetMultiChannelSetupData(
    const StreamDataParameters& stream_init_data,
    DMASetupData& stream_setup_data) {
  stream_setup_data.active_channel_count = stream_init_data.max_channel_count;
  int channel_id = 0;
  for (const auto& [address, records_per_channel] :
       stream_init_data.physical_addresses_map) {
    int records_so_far = 0;
    for (const auto& record_count : records_per_channel) {
      DMAChannelSetupData current_channel_setup_data{};
      current_channel_setup_data.record_count = record_count;
      current_channel_setup_data.channel_id = channel_id++;
      current_channel_setup_data.stream_address = reinterpret_cast<uintptr_t>(
          address + (stream_init_data.stream_record_size * records_so_far));
      stream_setup_data.channel_setup_data.push_back(
          current_channel_setup_data);
      records_so_far += record_count;
    }
  }
}

void DMASetup::SetSingleChannelSetupData(
    DMASetupData& stream_setup_data, const bool& is_input_stream,
    const StreamDataParameters& stream_init_data) {
  stream_setup_data.active_channel_count = -1;
  DMAChannelSetupData single_channel_stream_data{};
  if (is_input_stream) {
    single_channel_stream_data.record_count =
        stream_init_data.stream_record_count;
  } else {
    single_channel_stream_data.record_count = 0;
  }
  if (stream_init_data.physical_addresses_map.size() != 1 &&
      !stream_init_data.physical_addresses_map.begin()->second.empty()) {
    throw std::runtime_error("Incorrect addresses given!");
  }
  single_channel_stream_data.stream_address = reinterpret_cast<uintptr_t>(
      stream_init_data.physical_addresses_map.begin()->first);
  stream_setup_data.channel_setup_data.push_back(single_channel_stream_data);
}

void DMASetup::SetUpDMACrossbarsForStream(const DMASetupData& stream_setup_data,
                                          DMAInterface& dma_engine) {
  for (size_t current_chunk_index = 0;
       current_chunk_index < stream_setup_data.crossbar_setup_data.size();
       ++current_chunk_index) {
    // Order is reversed
    if (stream_setup_data.is_input_stream) {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dma_engine.SetCrossbarValues(
            module_config_values::DMACrossbarDirectionSelection::
                kBufferToInterfaceChunk,
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            {stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[3 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[2 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[1 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[current_offset * 4]});
        dma_engine.SetCrossbarValues(
            module_config_values::DMACrossbarDirectionSelection::
                kBufferToInterfacePosition,
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            {stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[3 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[2 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[1 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[current_offset * 4]});
      }
    } else {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dma_engine.SetCrossbarValues(
            module_config_values::DMACrossbarDirectionSelection::
                kInterfaceToBufferChunk,
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            {stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[3 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[2 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[1 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .chunk_selection[current_offset * 4]});
        dma_engine.SetCrossbarValues(
            module_config_values::DMACrossbarDirectionSelection::
                kInterfaceToBufferPosition,
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            {stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[3 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[2 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[1 + current_offset * 4],
             stream_setup_data.crossbar_setup_data[current_chunk_index]
                 .position_selection[current_offset * 4]});
      }
    }
  }
}

void DMASetup::SetUpDMAIOStream(const DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine) {
  dma_engine.SetControllerParams(
      stream_setup_data.is_input_stream, stream_setup_data.stream_id,
      stream_setup_data.ddr_burst_length,
      stream_setup_data.records_per_ddr_burst, stream_setup_data.buffer_start,
      stream_setup_data.buffer_end);
  if (stream_setup_data.is_input_stream) {
    dma_engine.SetRecordSize(stream_setup_data.stream_id,
                             stream_setup_data.chunks_per_record);
    for (const auto& chunk_id_pair : stream_setup_data.record_chunk_ids) {
      dma_engine.SetRecordChunkIDs(stream_setup_data.stream_id,
                                   std::get<0>(chunk_id_pair),
                                   std::get<1>(chunk_id_pair));
    }
    if (stream_setup_data.active_channel_count == -1) {
      dma_engine.SetControllerStreamAddress(
          true, stream_setup_data.stream_id,
          stream_setup_data.channel_setup_data[0].stream_address);
      dma_engine.SetControllerStreamSize(
          true, stream_setup_data.stream_id,
          stream_setup_data.channel_setup_data[0].record_count);
    } else {
      dma_engine.SetNumberOfActiveChannelsForMultiChannelStreams(
          stream_setup_data.stream_id, stream_setup_data.active_channel_count);

      for (const auto& channel_setup_data :
           stream_setup_data.channel_setup_data) {
        dma_engine.SetSizeForMultiChannelStreams(
            stream_setup_data.stream_id, channel_setup_data.channel_id,
            channel_setup_data.record_count);
        dma_engine.SetAddressForMultiChannelStreams(
            stream_setup_data.stream_id, channel_setup_data.channel_id,
            channel_setup_data.stream_address);
      }

      dma_engine.SetDDRBurstSizeForMultiChannelStreams(
          stream_setup_data.stream_id, stream_setup_data.ddr_burst_length);
      dma_engine.SetRecordsPerBurstForMultiChannelStreams(
          stream_setup_data.stream_id, stream_setup_data.records_per_ddr_burst);
    }
  } else {
    dma_engine.SetControllerStreamAddress(
        false, stream_setup_data.stream_id,
        stream_setup_data.channel_setup_data[0].stream_address);
    dma_engine.SetControllerStreamSize(
        false, stream_setup_data.stream_id,
        stream_setup_data.channel_setup_data[0].record_count);
  }
}
