#include "DMASetup.hpp"

#include <cmath>
#include <cstdio>
#include <queue>
#include <tuple>

#include "DMACrossbarSetup.hpp"

void DMASetup::SetupDMAModule(DMAInterface& dmaEngine, std::vector<int>& dbData,
                              int recordSize, int recordCount,
                              int inputStreamID, int outputStreamID) {
  // Calculate the controller parameter values based on input data and datatypes
  // Every size metric is 1 integer = 4 bytes = 32 bits
  const int max_ddr_burst_size = 512;
  const int max_chunk_size = 16;
  const int max_ddr_size_per_cycle = 4;

  // Input
  DMASetupData input_stream_setup_data;
  input_stream_setup_data.streamID = inputStreamID;
  input_stream_setup_data.isInputStream = true;
  input_stream_setup_data.recordCount = recordCount;
  // inputStreamSetupData.recordCount = doc.GetRowCount();
  CalculateDMAStreamSetupData(input_stream_setup_data, max_chunk_size,
                              max_ddr_burst_size, max_ddr_size_per_cycle,
                              dbData, recordSize);

  // Output
  DMASetupData output_stream_setup_data;
  output_stream_setup_data.streamID = outputStreamID;
  output_stream_setup_data.isInputStream = false;
  output_stream_setup_data.recordCount = 0;
  CalculateDMAStreamSetupData(output_stream_setup_data, max_chunk_size,
                              max_ddr_burst_size, max_ddr_size_per_cycle,
                              dbData, recordSize);

  const int any_chunk = 31;
  const int any_position = 3;
  DMACrossbarSetup crossbar_configuration_finder;
  DMACrossbarSetup::FindInputCrossbarSetupData(any_chunk, any_position,
                                               input_stream_setup_data);
  DMACrossbarSetup::FindOutputCrossbarSetupData(any_chunk, any_position,
                                                output_stream_setup_data);

  std::vector<DMASetupData> setup_data_for_dma;
  setup_data_for_dma.push_back(input_stream_setup_data);
  setup_data_for_dma.push_back(output_stream_setup_data);
  WriteSetupDataToDMAModule(setup_data_for_dma, dmaEngine);
}

void DMASetup::WriteSetupDataToDMAModule(
    std::vector<DMASetupData>& setupDataForDMA, DMAInterface& dmaEngine) {
  for (auto& stream_setup_data : setupDataForDMA) {
    SetUpDMAIOStreams(stream_setup_data, dmaEngine);
    SetUpDMACrossbars(stream_setup_data, dmaEngine);
  }
}

void DMASetup::SetUpDMACrossbars(DMASetupData& streamSetupData,
                                 DMAInterface& dmaEngine) {
  for (size_t current_chunk_index = 0;
       current_chunk_index < streamSetupData.crossbarSetupData.size();
       ++current_chunk_index) {
    if (streamSetupData.isInputStream) {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dmaEngine.setBufferToInterfaceChunk(
            streamSetupData.streamID, current_chunk_index, current_offset,
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[3 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[2 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[1 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[0 + current_offset * 4]);
        dmaEngine.setBufferToInterfaceSourcePosition(
            streamSetupData.streamID, current_chunk_index, current_offset,
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[3 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[2 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[1 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[0 + current_offset * 4]);
      }
    } else {
      for (int current_offset = 0; current_offset < 4; current_offset++) {
        dmaEngine.setInterfaceToBufferChunk(
            streamSetupData.streamID, current_chunk_index, current_offset,
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[3 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[2 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[1 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .chunkData[0 + current_offset * 4]);
        dmaEngine.setInterfaceToBufferSourcePosition(
            streamSetupData.streamID, current_chunk_index, current_offset,
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[3 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[2 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[1 + current_offset * 4],
            streamSetupData.crossbarSetupData[current_chunk_index]
                .positionData[0 + current_offset * 4]);
      }
    }
  }
}

void DMASetup::SetUpDMAIOStreams(DMASetupData& streamSetupData,
                                 DMAInterface& dmaEngine) {
  if (streamSetupData.isInputStream) {
    dmaEngine.setInputControllerParams(
        streamSetupData.streamID, streamSetupData.DDRBurstLength,
        streamSetupData.recordsPerDDRBurst, streamSetupData.bufferStart,
        streamSetupData.bufferEnd);
    dmaEngine.setInputControllerStreamAddress(streamSetupData.streamID,
                                              streamSetupData.streamAddress);
    dmaEngine.setInputControllerStreamSize(streamSetupData.streamID,
                                           streamSetupData.recordCount);
  } else {
    dmaEngine.setOutputControllerParams(
        streamSetupData.streamID, streamSetupData.DDRBurstLength,
        streamSetupData.recordsPerDDRBurst, streamSetupData.bufferStart,
        streamSetupData.bufferEnd);
    dmaEngine.setOutputControllerStreamAddress(streamSetupData.streamID,
                                               streamSetupData.streamAddress);
    dmaEngine.setOutputControllerStreamSize(streamSetupData.streamID,
                                            streamSetupData.recordCount);
  }
  dmaEngine.setRecordSize(streamSetupData.streamID,
                          streamSetupData.chunksPerRecord);
  for (auto& chunk_id_pair : streamSetupData.recordChunkIDs) {
    dmaEngine.setRecordChunkIDs(streamSetupData.streamID,
                                std::get<0>(chunk_id_pair),
                                std::get<1>(chunk_id_pair));
  }
}

void DMASetup::CalculateDMAStreamSetupData(DMASetupData& streamSetupData,
                                           const int& maxChunkSize,
                                           const int& maxDDRBurstSize,
                                           const int& maxDDRSizePerCycle,
                                           std::vector<int>& dbData,
                                           int recordSize) {
  streamSetupData.chunksPerRecord =
      (recordSize + maxChunkSize - 1) / maxChunkSize;  // ceil

  // Temporarily for now.
  for (int i = 0; i < streamSetupData.chunksPerRecord; i++) {
    streamSetupData.recordChunkIDs.emplace_back(i, i);
  }

  int records_per_max_burst_size = maxDDRBurstSize / recordSize;
  streamSetupData.recordsPerDDRBurst =
      pow(2, static_cast<int>(log2(records_per_max_burst_size)));

  streamSetupData.DDRBurstLength =
      ((recordSize * streamSetupData.recordsPerDDRBurst) + maxDDRSizePerCycle -
       1) /
      maxDDRSizePerCycle;  // ceil (recordSize * recordsPerDDRBurst) /
                           // maxDDRSizePerCycle

  // Temporarily for now
  streamSetupData.bufferStart = 0;
  streamSetupData.bufferEnd = 15;

  streamSetupData.streamAddress = reinterpret_cast<uintptr_t>(&dbData[0]);
}
