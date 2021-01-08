#include "dma_setup.hpp"

#include "dma_crossbar_setup.hpp"
#include "merge_sort_setup.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

auto DMASetup::CalculateMultiChannelStreamRecordCountPerChannel(
    int stream_record_count, int max_channel_count, int record_size) -> int {
  int channel_record_count =
      (stream_record_count + max_channel_count - 1) / max_channel_count;
  while (!((channel_record_count * record_size) % 16 == 0)) {
    channel_record_count++;
  }
  return channel_record_count;
}

void DMASetup::SetupDMAModule(
    DMAInterface& dma_engine,
    const std::vector<std::pair<StreamDataParameters, bool>>& streams,
    const bool is_input_stream) {
  const int buffer_size = 16 / streams.size();
  int multichannel_stream_count = 0;
  for (int current_stream_count = 0; current_stream_count < streams.size();
       current_stream_count++) {
    const StreamDataParameters stream_init_data =
        streams.at(current_stream_count).first;
    const bool is_multichannel_stream = streams.at(current_stream_count).second;

    DMASetupData stream_setup_data;
    if (!is_multichannel_stream) {
      if (is_input_stream) {
        stream_setup_data.record_count = stream_init_data.stream_record_count;
      } else {
        stream_setup_data.record_count = 0;
      }
      stream_setup_data.stream_address =
          reinterpret_cast<uintptr_t>(stream_init_data.physical_address);
    }

    stream_setup_data.is_input_stream = is_input_stream;

    stream_setup_data.stream_id = stream_init_data.stream_id;

    stream_setup_data.buffer_start = buffer_size * current_stream_count;
    stream_setup_data.buffer_end =
        stream_setup_data.buffer_start + buffer_size - 1;

    StreamParameterCalculator::CalculateDMAStreamSetupData(
        stream_setup_data, stream_init_data.stream_record_size,
        is_multichannel_stream);

    SetUpDMAIOStream(stream_setup_data, dma_engine, is_multichannel_stream);

    const int throwaway_chunk =
        query_acceleration_constants::kDatapathLength - 1;
    const int throwaway_position = 0;
    DMACrossbarSetup::CalculateCrossbarSetupData(
        throwaway_chunk, throwaway_position, stream_setup_data,
        stream_init_data.stream_record_size);

    SetUpDMACrossbarsForStream(stream_setup_data, dma_engine);

    if (is_multichannel_stream) {
      multichannel_stream_count++;
      DMAMultiChannelSetupData input_stream_setup_data;
      const int max_channel_count = 64 * 2;
      int channel_record_count =
          CalculateMultiChannelStreamRecordCountPerChannel(
              stream_init_data.stream_record_count, max_channel_count,
              stream_init_data.stream_record_size);

      input_stream_setup_data.active_channel_count =
          (stream_init_data.stream_record_count + channel_record_count - 1) /
          channel_record_count;
      input_stream_setup_data.channel_setup_data =
          std::vector<DMAChannelSetupData>();

      for (int j = 0; j < input_stream_setup_data.active_channel_count; j++) {
        DMAChannelSetupData current_channel_setup_data;
        if (j == input_stream_setup_data.active_channel_count - 1 &&
            stream_init_data.stream_record_count % channel_record_count != 0) {
          current_channel_setup_data.record_count =
              stream_init_data.stream_record_count % channel_record_count;
        } else {
          current_channel_setup_data.record_count = channel_record_count;
        }

        current_channel_setup_data.channel_id = j;
        current_channel_setup_data.stream_address = reinterpret_cast<uintptr_t>(
            stream_init_data.physical_address +
            (stream_init_data.stream_record_size * (channel_record_count * j)));

        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
      }

      // Just in case setting the unused channels to 0
      for (int j = input_stream_setup_data.active_channel_count;
           j < max_channel_count; j++) {
        DMAChannelSetupData current_channel_setup_data = {0, 0, j};
        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
      }

      /*dma_engine.SetNumberOfActiveChannelsForMultiChannelStreams(
        input_streams.at(i).stream_id,
        input_stream_setup_data.active_channel_count);*/
      dma_engine.SetNumberOfActiveChannelsForMultiChannelStreams(
          stream_init_data.stream_id, max_channel_count);

      for (const auto& channel_setup_data :
           input_stream_setup_data.channel_setup_data) {
        dma_engine.SetSizeForMultiChannelStreams(
            stream_init_data.stream_id, channel_setup_data.channel_id,
            channel_setup_data.record_count);
        dma_engine.SetAddressForMultiChannelStreams(
            stream_init_data.stream_id, channel_setup_data.channel_id,
            channel_setup_data.stream_address);
      }

      dma_engine.SetDDRBurstSizeForMultiChannelStreams(
          stream_init_data.stream_id, stream_setup_data.ddr_burst_length);
      dma_engine.SetRecordsPerBurstForMultiChannelStreams(
          stream_init_data.stream_id, stream_setup_data.records_per_ddr_burst);
    }
  }

  if (is_input_stream && multichannel_stream_count != 0) {
    dma_engine.SetNumberOfInputStreamsWithMultipleChannels(
        multichannel_stream_count);
  }
}

void DMASetup::SetUpDMACrossbarsForStream(const DMASetupData& stream_setup_data,
                                          DMAInterface& dma_engine) {
  for (size_t current_chunk_index = 0;
       current_chunk_index < stream_setup_data.crossbar_setup_data.size();
       ++current_chunk_index) {
    if (stream_setup_data.is_input_stream) {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dma_engine.SetBufferToInterfaceChunk(
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[3 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[2 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[1 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[0 + current_offset * 4]);
        dma_engine.SetBufferToInterfaceSourcePosition(
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[3 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[2 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[1 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[0 + current_offset * 4]);
      }
    } else {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dma_engine.SetInterfaceToBufferChunk(
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[3 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[2 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[1 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .chunk_data[0 + current_offset * 4]);
        dma_engine.SetInterfaceToBufferSourcePosition(
            stream_setup_data.stream_id, current_chunk_index, current_offset,
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[3 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[2 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[1 + current_offset * 4],
            stream_setup_data.crossbar_setup_data[current_chunk_index]
                .position_data[0 + current_offset * 4]);
      }
    }
  }
}

void DMASetup::SetUpDMAIOStream(const DMASetupData& stream_setup_data,
                                DMAInterface& dma_engine,
                                bool is_multichannel_stream) {
  if (stream_setup_data.is_input_stream) {
    dma_engine.SetInputControllerParams(
        stream_setup_data.stream_id, stream_setup_data.ddr_burst_length,
        stream_setup_data.records_per_ddr_burst, stream_setup_data.buffer_start,
        stream_setup_data.buffer_end);
    dma_engine.SetRecordSize(stream_setup_data.stream_id,
                             stream_setup_data.chunks_per_record);
    for (auto& chunk_id_pair : stream_setup_data.record_chunk_ids) {
      dma_engine.SetRecordChunkIDs(stream_setup_data.stream_id,
                                   std::get<0>(chunk_id_pair),
                                   std::get<1>(chunk_id_pair));
    }
    if (!is_multichannel_stream) {
      dma_engine.SetInputControllerStreamAddress(
          stream_setup_data.stream_id, stream_setup_data.stream_address);
      dma_engine.SetInputControllerStreamSize(stream_setup_data.stream_id,
                                              stream_setup_data.record_count);
    }
  } else {
    dma_engine.SetOutputControllerParams(
        stream_setup_data.stream_id, stream_setup_data.ddr_burst_length,
        stream_setup_data.records_per_ddr_burst, stream_setup_data.buffer_start,
        stream_setup_data.buffer_end);
    dma_engine.SetOutputControllerStreamAddress(
        stream_setup_data.stream_id, stream_setup_data.stream_address);
    dma_engine.SetOutputControllerStreamSize(stream_setup_data.stream_id,
                                             stream_setup_data.record_count);
  }
}
