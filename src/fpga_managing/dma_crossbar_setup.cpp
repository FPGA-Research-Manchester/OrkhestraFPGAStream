#include "dma_crossbar_setup.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>

#include "dma_crossbar_setup_data.hpp"
#include "dma_crossbar_specifier.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

using namespace dbmstodspi::fpga_managing;

// TODO(Kaspar): Rething record_size since it isn't used for output
void DMACrossbarSetup::CalculateCrossbarSetupData(
    DMASetupData& stream_setup_data, const int record_size,
    const std::vector<int>& selected_columns) {
  std::vector<int> expanded_column_selection;

  if (stream_setup_data.is_input_stream) {
    ConfigureInputCrossbarSetupData(selected_columns, stream_setup_data,
                                    expanded_column_selection, record_size);
  } else {
    ConfigureOutputCrossbarSetupData(
        selected_columns, expanded_column_selection, stream_setup_data);
  }

  //PrintCrossbarConfigData(expanded_column_selection, stream_setup_data);
}

auto DMACrossbarSetup::GetReverseIndex(int index, int row_size) -> int {
  int current_chunk = index / row_size;
  int current_pos = row_size - 1 - (index % row_size);
  return current_chunk * row_size + current_pos;
}

void DMACrossbarSetup::SetUpEmptyCrossbarSetupData(
    DMASetupData& stream_setup_data, const int required_chunk_count) {
  for (int current_chunk_count = stream_setup_data.crossbar_setup_data.size();
       current_chunk_count < required_chunk_count; current_chunk_count++) {
    stream_setup_data.crossbar_setup_data.emplace_back(DMACrossbarSetupData());
  }
}

void DMACrossbarSetup::FillSetupDataWithNegativePositions(
    DMASetupData& stream_setup_data) {
  for (auto& chunk_index : stream_setup_data.crossbar_setup_data) {
    for (int& position_index : chunk_index.chunk_selection) {
      position_index = -1;
    }
  }
}

// Current location - Which position we are currently configuring
// Target location - What data do we want to place at the current location
void DMACrossbarSetup::SetNextInputConfiguration(
    DMASetupData& stream_setup_data, const int current_location,
    const int target_location) {
  // 1st vertical pulling
  stream_setup_data
      .crossbar_setup_data[current_location /
                           query_acceleration_constants::kDatapathWidth]
      .chunk_selection[target_location %
                       query_acceleration_constants::kDatapathWidth] =
      target_location / query_acceleration_constants::kDatapathWidth;
  // 2nd horisontal pulling
  stream_setup_data
      .crossbar_setup_data[current_location /
                           query_acceleration_constants::kDatapathWidth]
      .position_selection[current_location %
                          query_acceleration_constants::kDatapathWidth] =
      target_location % query_acceleration_constants::kDatapathWidth;
}
void DMACrossbarSetup::SetNextOutputConfiguration(
    DMASetupData& stream_setup_data, const int current_location,
    const int target_location) {
  // 1st horisontal pulling
  stream_setup_data
      .crossbar_setup_data[target_location /
                           query_acceleration_constants::kDatapathWidth]
      .position_selection[current_location %
                          query_acceleration_constants::kDatapathWidth] =
      target_location % query_acceleration_constants::kDatapathWidth;
  // 2nd vertical pushing
  stream_setup_data
      .crossbar_setup_data[target_location /
                           query_acceleration_constants::kDatapathWidth]
      .chunk_selection[current_location %
                       query_acceleration_constants::kDatapathWidth] =
      current_location / query_acceleration_constants::kDatapathWidth;
}

auto DMACrossbarSetup::CreateFreeChunksVector() -> std::vector<int> {
  std::vector<int> free_chunks(query_acceleration_constants::kDatapathLength);
  std::iota(free_chunks.begin(), free_chunks.end(), 0);
  std::reverse(free_chunks.begin(), free_chunks.end());
  return free_chunks;
}

void DMACrossbarSetup::MarkChunksAsUsed(const DMASetupData& stream_setup_data,
                                        const int column_id,
                                        std::vector<int>& free_chunks) {
  for (const auto& chunk_id : stream_setup_data.crossbar_setup_data) {
    if (chunk_id.chunk_selection[column_id] != -1) {
      auto find_iterator = std::find(free_chunks.begin(), free_chunks.end(),
                                     chunk_id.chunk_selection[column_id]);
      if (find_iterator != free_chunks.end()) {
        free_chunks.erase(find_iterator);
      }
    }
  }
}

void DMACrossbarSetup::AllocateAvailableChunks(
    DMASetupData& stream_setup_data, const int column_id,
    const std::vector<int>& free_chunks) {
  for (auto& chunk_id : stream_setup_data.crossbar_setup_data) {
    if (chunk_id.chunk_selection[column_id] == -1) {
      chunk_id.chunk_selection[column_id] = free_chunks.at(0);
    }
  }
}

void DMACrossbarSetup::InsertMissingEmptySetupChunks(
    DMASetupData& stream_setup_data, const int missing_chunk_count_per_record) {
  for (int missing_chunk_index = stream_setup_data.crossbar_setup_data.size();
       missing_chunk_index > 0;
       missing_chunk_index -= stream_setup_data.chunks_per_record) {
    stream_setup_data.crossbar_setup_data.insert(
        stream_setup_data.crossbar_setup_data.begin() + missing_chunk_index,
        missing_chunk_count_per_record, DMACrossbarSetupData());
  }
}

void DMACrossbarSetup::ConfigureInputCrossbarSetupData(
    const std::vector<int>& selected_columns, DMASetupData& stream_setup_data,
    std::vector<int>& expanded_column_selection, const int& record_size) {
  assert(!DMACrossbarSpecifier::IsInputClashing(selected_columns));
  if (stream_setup_data.active_channel_count == -1) {
    expanded_column_selection =
        DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
            record_size, selected_columns,
            stream_setup_data.records_per_ddr_burst);
  } else {
    expanded_column_selection =
        DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
            record_size, selected_columns,
            stream_setup_data.records_per_ddr_burst,
            stream_setup_data.chunks_per_record);
  }

  SetUpEmptyCrossbarSetupData(stream_setup_data,
                              expanded_column_selection.size() /
                                  query_acceleration_constants::kDatapathWidth);
  for (int current_selection_id = 0;
       current_selection_id < expanded_column_selection.size();
       current_selection_id++) {
    if (expanded_column_selection[current_selection_id] != -1) {
      SetNextInputConfiguration(
          stream_setup_data, current_selection_id,
          expanded_column_selection[current_selection_id]);
    }
  }
}

void DMACrossbarSetup::ConfigureOutputCrossbarSetupData(
    const std::vector<int>& selected_columns,
    std::vector<int>& expanded_column_selection,
    DMASetupData& stream_setup_data) {
  assert(!DMACrossbarSpecifier::IsOutputClashing(selected_columns));
  expanded_column_selection = DMACrossbarSpecifier::ExtendOutputSpecification(
      selected_columns, stream_setup_data.records_per_ddr_burst,
      stream_setup_data.chunks_per_record);
  SetUpEmptyCrossbarSetupData(stream_setup_data,
                              stream_setup_data.chunks_per_record *
                                  stream_setup_data.records_per_ddr_burst);

  // Check if the default 31st chunk selection can be used
  bool needs_overwrite_check =
      expanded_column_selection.size() -
          std::count(expanded_column_selection.begin(),
                     expanded_column_selection.end(), -2) >
      query_acceleration_constants::kDdrBurstSize -
          query_acceleration_constants::kDatapathWidth;

  if (needs_overwrite_check) {
    FillSetupDataWithNegativePositions(stream_setup_data);
  }

  int post_record_junk_data_count = 0;
  for (int selection_id = 0; selection_id < expanded_column_selection.size();
       selection_id++) {
    const int current_reversed_selection_id = GetReverseIndex(
        selection_id, query_acceleration_constants::kDatapathWidth);
    if (expanded_column_selection[current_reversed_selection_id] == -1) {
      // Do nothing
    } else if (expanded_column_selection[current_reversed_selection_id] == -2) {
      post_record_junk_data_count++;
    } else {
      SetNextOutputConfiguration(
          stream_setup_data,
          GetReverseIndex(selection_id - post_record_junk_data_count,
                          query_acceleration_constants::kDatapathWidth),
          expanded_column_selection[current_reversed_selection_id]);
    }
  }

  if (needs_overwrite_check) {
    for (int column_id = 0;
         column_id < query_acceleration_constants::kDatapathWidth;
         column_id++) {
      std::vector<int> free_chunks = CreateFreeChunksVector();
      MarkChunksAsUsed(stream_setup_data, column_id, free_chunks);
      AllocateAvailableChunks(stream_setup_data, column_id, free_chunks);
    }
  } else {
    const int next_power_of_two = StreamParameterCalculator::FindNextPowerOfTwo(
        stream_setup_data.chunks_per_record);
    int missing_chunk_count_per_record =
        next_power_of_two - stream_setup_data.chunks_per_record;
    if (missing_chunk_count_per_record != 0) {
      InsertMissingEmptySetupChunks(stream_setup_data,
                                    missing_chunk_count_per_record);
    }
  }
}

void DMACrossbarSetup::PrintCrossbarConfigData(
    const std::vector<int>& expanded_column_selection,
    const DMASetupData& stream_setup_data) {
  std::cout << std::endl << "Specification:" << std::endl;
  for (int i = 0; i < expanded_column_selection.size(); i++) {
    if (i % query_acceleration_constants::kDatapathWidth == 0) {
      std::cout << std::endl;
    }
    std::cout << expanded_column_selection.at(i) << " ";
  }

  std::cout << std::endl << "Position selection:" << std::endl;
  for (const auto& chunk_setup : stream_setup_data.crossbar_setup_data) {
    std::cout << std::endl;
    for (const auto& position_select_data : chunk_setup.position_selection) {
      std::cout << position_select_data << " ";
    }
  }

  std::cout << std::endl << "Chunk selection:" << std::endl;
  for (const auto& chunk_setup : stream_setup_data.crossbar_setup_data) {
    std::cout << std::endl;
    for (const auto& chunk_select_data : chunk_setup.chunk_selection) {
      std::cout << chunk_select_data << " ";
    }
  }
}