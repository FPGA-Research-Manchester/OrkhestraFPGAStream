#include "dma_setup.hpp"
#include "query_acceleration_constants.hpp"
#include "dma_crossbar_setup.hpp"
#include "stream_parameter_calculator.hpp"

void DMASetup::SetupDMAModule(DMAInterface& dma_engine,
                              volatile uint32_t* input_memory_address,
                              volatile uint32_t* output_memory_address,
                              const int record_size, const int record_count,
                              const int input_stream_id,
                              const int output_stream_id) {
  // Calculate the controller parameter values based on input data and datatypes
  // Every size metric is 1 integer = 4 bytes = 32 bits
  const int max_ddr_burst_size = query_acceleration_constants::kDdrBurstSize;
  const int max_chunk_size = query_acceleration_constants::kDatapathWidth;
  const int max_ddr_size_per_cycle =
      query_acceleration_constants::kDdrSizePerCycle;

  // Input
  DMASetupData input_stream_setup_data;
  input_stream_setup_data.stream_id = input_stream_id;
  input_stream_setup_data.is_input_stream = true;
  input_stream_setup_data.record_count = record_count;
  // inputStreamSetupData.record_count = doc.GetRowCount();
  StreamParameterCalculator::CalculateDMAStreamSetupData(input_stream_setup_data, max_chunk_size,
                              max_ddr_burst_size, max_ddr_size_per_cycle,
                              input_memory_address, record_size);

  // Output
  DMASetupData output_stream_setup_data;
  output_stream_setup_data.stream_id = output_stream_id;
  output_stream_setup_data.is_input_stream = false;
  output_stream_setup_data.record_count = 0;
  StreamParameterCalculator::CalculateDMAStreamSetupData(
      output_stream_setup_data, max_chunk_size,
                              max_ddr_burst_size, max_ddr_size_per_cycle,
                              output_memory_address, record_size);

  const int any_chunk = 31;
  const int any_position = 3;
  DMACrossbarSetup::CalculateCrossbarSetupData(any_chunk, any_position, input_stream_setup_data, record_size);
  DMACrossbarSetup::CalculateCrossbarSetupData(any_chunk, any_position, output_stream_setup_data, record_size);

  std::vector<DMASetupData> setup_data_for_dma;
  setup_data_for_dma.push_back(input_stream_setup_data);
  setup_data_for_dma.push_back(output_stream_setup_data);
  WriteSetupDataToDMAModule(setup_data_for_dma, dma_engine);
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
