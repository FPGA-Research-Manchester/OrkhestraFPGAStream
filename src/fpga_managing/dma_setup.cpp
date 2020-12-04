#include "dma_setup.hpp"

#include "dma_crossbar_setup.hpp"
#include "dma_multi_channel_setup_data.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

void DMASetup::SetupDMAModuleWithMultiStream(
    DMAInterface& dma_engine,
    const std::vector<StreamDataParameters>& input_streams,
    const std::vector<StreamDataParameters>& output_streams) {
  std::vector<DMASetupData> setup_output_data_for_dma;
  int buffer_size = 16 / output_streams.size();
  for (int i = 0; i < output_streams.size(); i++) {
    DMASetupData output_stream_setup_data;
    output_stream_setup_data.is_input_stream = false;
    output_stream_setup_data.record_count = 0;

    AddNewStreamDMASetupData(output_stream_setup_data, output_streams.at(i),
                             buffer_size, setup_output_data_for_dma, i);
  }

  WriteSetupDataToDMAModule(setup_output_data_for_dma, dma_engine);

  dma_engine.SetNumberOfInputStreamsWithMultipleChannels(input_streams.size());

  for (int i = 0; i < input_streams.size(); i++) {
    DMAMultiChannelSetupData input_stream_setup_data;
    input_stream_setup_data.is_input_stream = false;

    input_stream_setup_data.stream_id = input_streams.at(i).stream_id;
    input_stream_setup_data.buffer_start = buffer_size * i;
    input_stream_setup_data.buffer_end =
        input_stream_setup_data.buffer_start + buffer_size - 1;

    // Placeholder used for now. Some inheritance needed ASAP here
    DMASetupData setup_data_placeholder;

    StreamParameterCalculator::CalculateDMAStreamSetupData(
        setup_data_placeholder, query_acceleration_constants::kDatapathWidth,
        query_acceleration_constants::kDdrBurstSize,
        query_acceleration_constants::kDdrSizePerCycle,
        input_streams.at(i).stream_record_size);

    input_stream_setup_data.chunks_per_record =
        setup_data_placeholder.chunks_per_record;
    input_stream_setup_data.record_chunk_ids =
        setup_data_placeholder.record_chunk_ids;
    input_stream_setup_data.records_per_ddr_burst =
        setup_data_placeholder.records_per_ddr_burst;
    input_stream_setup_data.ddr_burst_length =
        setup_data_placeholder.ddr_burst_length;

    const int throwaway_chunk =
        query_acceleration_constants::kDatapathLength - 1;
    const int throwaway_position = 0;

    setup_data_placeholder.is_input_stream = true;

    DMACrossbarSetup::CalculateCrossbarSetupData(
        throwaway_chunk, throwaway_position, setup_data_placeholder,
        input_streams.at(i).stream_record_size);

    // Useless
    input_stream_setup_data.crossbar_setup_data =
        setup_data_placeholder.crossbar_setup_data;

    input_stream_setup_data.active_channel_count = 64;

    for (int j = 0; j < input_stream_setup_data.active_channel_count; j++) {
      if (j <= 60) {
        int channel_record_count =
            (input_streams.at(i).stream_record_count +
             input_stream_setup_data.active_channel_count) -
            1 / input_stream_setup_data.active_channel_count;
        DMAChannelSetupData current_channel_setup_data{
            reinterpret_cast<uintptr_t>(
                input_streams.at(i).physical_address +
                (input_streams.at(i).stream_record_size *
                 (channel_record_count * j))),
            channel_record_count, j};
        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
        // Remove this junk after!
      } else if (j == 61) {
        int channel_record_count = 7;
        DMAChannelSetupData current_channel_setup_data{
            reinterpret_cast<uintptr_t>(
                input_streams.at(i).physical_address +
                (input_streams.at(i).stream_record_size * 975)),
            channel_record_count, j};
        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
      } else if (j == 62) {
        int channel_record_count = 10;
        DMAChannelSetupData current_channel_setup_data{
            reinterpret_cast<uintptr_t>(
                input_streams.at(i).physical_address +
                (input_streams.at(i).stream_record_size * 982)),
            channel_record_count, j};
        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
      } else if (j == 63) {
        int channel_record_count = 7;
        DMAChannelSetupData current_channel_setup_data{
            reinterpret_cast<uintptr_t>(
                input_streams.at(i).physical_address +
                (input_streams.at(i).stream_record_size * 992)),
            channel_record_count, j};
        input_stream_setup_data.channel_setup_data.push_back(
            current_channel_setup_data);
      }
    }

    SetUpDMACrossbars(setup_data_placeholder, dma_engine);

    dma_engine.SetInputControllerParams(
        input_stream_setup_data.stream_id,
        input_stream_setup_data.ddr_burst_length,
        input_stream_setup_data.records_per_ddr_burst,
        input_stream_setup_data.buffer_start,
        input_stream_setup_data.buffer_end);
    dma_engine.SetRecordSize(input_stream_setup_data.stream_id,
                             input_stream_setup_data.chunks_per_record);
    for (const auto& chunk_id_pair : input_stream_setup_data.record_chunk_ids) {
      dma_engine.SetRecordChunkIDs(input_stream_setup_data.stream_id,
                                   std::get<0>(chunk_id_pair),
                                   std::get<1>(chunk_id_pair));
    }

    dma_engine.SetNumberOfActiveChannelsForMultiChannelStreams(
        input_streams.at(i).stream_id,
        input_stream_setup_data.active_channel_count);

    for (const auto &channel_setup_data : input_stream_setup_data.channel_setup_data) {
      dma_engine.SetSizeForMultiChannelStreams(input_streams.at(i).stream_id,
                                               channel_setup_data.channel_id,
                                               channel_setup_data.record_count);
      dma_engine.SetAddressForMultiChannelStreams(
          input_streams.at(i).stream_id, channel_setup_data.channel_id,
          channel_setup_data.stream_address);
    }

    dma_engine.SetDDRBurstSizeForMultiChannelStreams(
        input_streams.at(i).stream_id,
        input_stream_setup_data.ddr_burst_length);
    dma_engine.SetRecordsPerBurstForMultiChannelStreams(
        input_streams.at(i).stream_id,
        input_stream_setup_data.records_per_ddr_burst);
  }
}

void DMASetup::SetupDMAModule(
    DMAInterface& dma_engine,
    const std::vector<StreamDataParameters>& input_streams,
    const std::vector<StreamDataParameters>& output_streams) {
  std::vector<DMASetupData> setup_data_for_dma;

  int buffer_size = 16 / input_streams.size();
  for (int i = 0; i < input_streams.size(); i++) {
    DMASetupData input_stream_setup_data;
    input_stream_setup_data.is_input_stream = true;
    input_stream_setup_data.record_count = input_streams[i].stream_record_count;

    AddNewStreamDMASetupData(input_stream_setup_data, input_streams.at(i),
                             buffer_size, setup_data_for_dma, i);
  }

  buffer_size = 16 / output_streams.size();
  for (int i = 0; i < output_streams.size(); i++) {
    DMASetupData output_stream_setup_data;
    output_stream_setup_data.is_input_stream = false;
    output_stream_setup_data.record_count = 0;

    AddNewStreamDMASetupData(output_stream_setup_data, output_streams.at(i),
                             buffer_size, setup_data_for_dma, i);
  }

  WriteSetupDataToDMAModule(setup_data_for_dma, dma_engine);
}

void DMASetup::AddNewStreamDMASetupData(
    DMASetupData& stream_setup_data,
    const StreamDataParameters& stream_init_data, int buffer_size,
    std::vector<DMASetupData>& setup_data_for_dma, int current_stream_count) {
  stream_setup_data.stream_id = stream_init_data.stream_id;
  stream_setup_data.buffer_start = buffer_size * current_stream_count;
  stream_setup_data.buffer_end =
      stream_setup_data.buffer_start + buffer_size - 1;

  StreamParameterCalculator::CalculateDMAStreamSetupData(
      stream_setup_data, query_acceleration_constants::kDatapathWidth,
      query_acceleration_constants::kDdrBurstSize,
      query_acceleration_constants::kDdrSizePerCycle,
      stream_init_data.stream_record_size);

  const int throwaway_chunk = query_acceleration_constants::kDatapathLength - 1;
  const int throwaway_position = 0;

  DMACrossbarSetup::CalculateCrossbarSetupData(
      throwaway_chunk, throwaway_position, stream_setup_data,
      stream_init_data.stream_record_size);

  stream_setup_data.stream_address =
      reinterpret_cast<uintptr_t>(stream_init_data.physical_address);

  setup_data_for_dma.push_back(stream_setup_data);
}

void DMASetup::WriteSetupDataToDMAModule(
    std::vector<DMASetupData>& setup_data_for_dma, DMAInterface& dma_engine) {
  for (auto& stream_setup_data : setup_data_for_dma) {
    SetUpDMAIOStreams(stream_setup_data, dma_engine);
    SetUpDMACrossbars(stream_setup_data, dma_engine);
  }
}

void DMASetup::SetUpDMACrossbars(DMASetupData& stream_setup_data,
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

void DMASetup::SetUpDMAIOStreams(DMASetupData& stream_setup_data,
                                 DMAInterface& dma_engine) {
  if (stream_setup_data.is_input_stream) {
    dma_engine.SetInputControllerParams(
        stream_setup_data.stream_id, stream_setup_data.ddr_burst_length,
        stream_setup_data.records_per_ddr_burst, stream_setup_data.buffer_start,
        stream_setup_data.buffer_end);
    dma_engine.SetInputControllerStreamAddress(
        stream_setup_data.stream_id, stream_setup_data.stream_address);
    dma_engine.SetInputControllerStreamSize(stream_setup_data.stream_id,
                                            stream_setup_data.record_count);
    dma_engine.SetRecordSize(stream_setup_data.stream_id,
                             stream_setup_data.chunks_per_record);
    for (auto& chunk_id_pair : stream_setup_data.record_chunk_ids) {
      dma_engine.SetRecordChunkIDs(stream_setup_data.stream_id,
                                   std::get<0>(chunk_id_pair),
                                   std::get<1>(chunk_id_pair));
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
