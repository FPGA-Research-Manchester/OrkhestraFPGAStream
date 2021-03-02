#include "dma_crossbar_setup.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>

#include "dma_crossbar_setup_data.hpp"
#include "dma_crossbar_specifier.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

// Get rid of stream_setup_data and record_size
void DMACrossbarSetup::CalculateCrossbarSetupData(
    DMASetupData& stream_setup_data, int record_size,
    std::vector<int> selected_columns) {
  // std::queue<int> source_chunks;
  // std::queue<int> target_positions;

  std::vector<int> expanded_column_selection;

  if (stream_setup_data.is_input_stream) {
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
    // This assert should be moved inside the extension methods. And it should
    // check that no number goes over 512!
    assert(expanded_column_selection.size() <=
           query_acceleration_constants::kDdrBurstSize);

    for (int current_chunk_count = stream_setup_data.crossbar_setup_data.size();
         current_chunk_count < expanded_column_selection.size() /
                                   query_acceleration_constants::kDatapathWidth;
         current_chunk_count++) {
      stream_setup_data.crossbar_setup_data.emplace_back(
          DMACrossbarSetupData());
    }
    for (int current_selection_id = 0;
         current_selection_id < expanded_column_selection.size();
         current_selection_id++) {
      if (expanded_column_selection[current_selection_id] != -1) {
        stream_setup_data
            .crossbar_setup_data[current_selection_id /
                                 query_acceleration_constants::kDatapathWidth]
            .chunk_selection[expanded_column_selection[current_selection_id] %
                             query_acceleration_constants::kDatapathWidth] =
            expanded_column_selection[current_selection_id] /
            query_acceleration_constants::kDatapathWidth;

        stream_setup_data
            .crossbar_setup_data[current_selection_id /
                                 query_acceleration_constants::kDatapathWidth]
            .position_selection[current_selection_id %
                                query_acceleration_constants::kDatapathWidth] =
            expanded_column_selection[current_selection_id] %
            query_acceleration_constants::kDatapathWidth;
      }
    }

  } else {
    assert(!DMACrossbarSpecifier::IsOutputClashing(selected_columns));
    expanded_column_selection = DMACrossbarSpecifier::ExtendOutputSpecification(
        selected_columns, stream_setup_data.records_per_ddr_burst,
        stream_setup_data.chunks_per_record);
    for (int current_chunk_count = stream_setup_data.crossbar_setup_data.size();
         current_chunk_count < expanded_column_selection.size() /
                                   query_acceleration_constants::kDatapathWidth;
         current_chunk_count++) {
      stream_setup_data.crossbar_setup_data.emplace_back(
          DMACrossbarSetupData());
    }
    // record_size tells us the location where garbage data starts at the last chunk of the record
    for (int current_selection_id = 0;
         current_selection_id < expanded_column_selection.size();
         current_selection_id++) {
      if (expanded_column_selection[current_selection_id] != -1) {
        // expanded_column_selection has to be changed to show where integers go to rather than where they come from.
      }
    }
  }

  for (const auto& thing : stream_setup_data.crossbar_setup_data) {
    std::cout << std::endl;
    for (const auto& thing2 : thing.position_selection) {
      std::cout << thing2 << " ";
    }
  }

  // int selected_record_size = selected_columns.size();
  // int chunks_per_record =
  //    StreamParameterCalculator::CalculateChunksPerRecord(selected_record_size);

  //// Currently selected columns is expanded to the full buffer size.
  //// Later this can be made into the actual size.
  // std::vector<int> expanded_column_selection;
  // for (int i = 0; i < query_acceleration_constants::kDdrBurstSize * 10; i++)
  // {
  //  expanded_column_selection.push_back(
  //      selected_columns[i % selected_columns.size()] +
  //      (record_size * (i / selected_columns.size())));
  //}

  // auto selected_columns_iterator = expanded_column_selection.begin();
  // int rows_processed = 0;

  // const int last_chunk_leftover_size =
  //    selected_record_size % query_acceleration_constants::kDatapathWidth;

  // if (last_chunk_leftover_size == 0) {
  //  for (int chunk_id = 0;
  //       chunk_id < query_acceleration_constants::kDatapathLength; chunk_id++)
  //       {
  //    if (chunk_id % chunks_per_record == 0 &&
  //        chunk_id != 0) {
  //      AddBubbleChunkAndPositionData(source_chunks, target_positions,
  //                                    chunks_per_record,
  //                                    31, 0);
  //    }
  //    for (int position_id = 0;
  //         position_id < query_acceleration_constants::kDatapathWidth;
  //         position_id++) {
  //      int column_number = *selected_columns_iterator;
  //      source_chunks.push(column_number /
  //                         query_acceleration_constants::kDatapathWidth);
  //      target_positions.push(column_number %
  //                            query_acceleration_constants::kDatapathWidth);
  //      selected_columns_iterator++;
  //    }
  //  }
  //} else {
  //  const int steps_per_cycle =
  //      query_acceleration_constants::kDatapathWidth /
  //      std::gcd(selected_record_size,
  //      query_acceleration_constants::kDatapathWidth);
  //  const int cycle_count = std::ceil(
  //      static_cast<float>(query_acceleration_constants::kDatapathLength) /
  //      static_cast<float>(steps_per_cycle * chunks_per_record));

  //  const float cycle_step_chunk_increase =
  //      static_cast<float>(selected_record_size) /
  //      static_cast<float>(query_acceleration_constants::kDatapathWidth);

  //  if (stream_setup_data.is_input_stream) {
  //    CalculateBufferToInterfaceSetupConfig(
  //        source_chunks, target_positions, 31, 0,
  //        last_chunk_leftover_size, steps_per_cycle, cycle_count,
  //        chunks_per_record, cycle_step_chunk_increase,
  //        selected_columns_iterator);
  //  } else {
  //    CalculateInterfaceToBufferSetupConfig(
  //        source_chunks, target_positions, 31, 0,
  //        last_chunk_leftover_size, steps_per_cycle, cycle_count,
  //        chunks_per_record, cycle_step_chunk_increase,
  //        selected_columns_iterator);
  //  }
  //}

  // SetCrossbarSetupDataForStream(source_chunks, target_positions,
  //                              stream_setup_data);
}

// Input
void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position,
    const int& last_chunk_leftover_size, const int& steps_per_cycle,
    const int& cycle_count, const int& chunks_per_record,
    const float& cycle_step_chunk_increase,
    std::vector<int>::iterator& selected_columns_iterator) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int used_leftover_chunks_count =
          cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth -
          (used_leftover_chunks_count %
           query_acceleration_constants::kDatapathWidth);

      const int current_cycle_step_initial_chunk =
          cycle_step_chunk_increase *
          (cycle_counter * steps_per_cycle + cycle_step);

      InitialDataSetupFromBufferToInterface(
          chunks_per_record, current_cycle_step_initial_chunk,
          current_offset_point, source_chunks, target_positions,
          selected_columns_iterator);

      FinalDataSetupFromBufferToInterface(
          used_leftover_chunks_count, current_cycle_step_initial_chunk,
          current_offset_point, last_chunk_leftover_size, chunks_per_record,
          source_chunks, any_chunk, target_positions, any_position,
          selected_columns_iterator);

      AddBubbleChunkAndPositionData(source_chunks, target_positions,
                                    chunks_per_record, any_chunk, any_position);
    }
  }
}

// Output
void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position,
    const int& last_chunk_leftover_size, const int& steps_per_cycle,
    const int& cycle_count, const int& chunks_per_record,
    const float& cycle_step_chunk_increase,
    std::vector<int>::iterator& selected_columns_iterator) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int used_leftover_chunks_count =
          cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth -
          (used_leftover_chunks_count %
           query_acceleration_constants::kDatapathWidth);

      const int current_cycle_step_initial_chunk =
          cycle_step_chunk_increase *
          (cycle_counter * steps_per_cycle + cycle_step);

      InitialDataSetupFromInterfaceToBuffer(
          chunks_per_record, current_cycle_step_initial_chunk,
          current_offset_point,
          used_leftover_chunks_count %
              query_acceleration_constants::kDatapathWidth,
          source_chunks, target_positions, selected_columns_iterator);

      FinalDataSetupFromInterfaceToBuffer(
          used_leftover_chunks_count, current_cycle_step_initial_chunk,
          last_chunk_leftover_size, chunks_per_record, source_chunks, any_chunk,
          target_positions, any_position, selected_columns_iterator);

      AddBubbleChunkAndPositionData(source_chunks, target_positions,
                                    chunks_per_record, any_chunk, any_position);
    }
  }
}

void DMACrossbarSetup::InitialDataSetupFromBufferToInterface(
    const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, std::queue<int>& source_chunks,
    std::queue<int>& target_positions,
    std::vector<int>::iterator& selected_columns_iterator) {
  for (int initial_chunk_count = 0; initial_chunk_count < chunks_per_record - 1;
       initial_chunk_count++) {
    const int current_chunk =
        current_cycle_step_initial_chunk + initial_chunk_count;
    InitialChunkSetupFromBufferToInterface(
        current_offset_point, source_chunks, current_chunk,
        std::vector<int>::iterator(selected_columns_iterator));
    InitialPositionSetupFromBufferToInterface(
        current_offset_point, target_positions, selected_columns_iterator);
  }
}

void DMACrossbarSetup::FinalDataSetupFromBufferToInterface(
    const int& used_leftover_chunks_count,
    const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, const int& last_chunk_leftover_size,
    const int& chunks_per_record, std::queue<int>& source_chunks,
    const int& any_chunk, std::queue<int>& target_positions,
    const int& any_position,
    std::vector<int>::iterator& selected_columns_iterator) {
  FinalChunkSetupFromBufferToInterface(
      used_leftover_chunks_count, last_chunk_leftover_size,
      current_cycle_step_initial_chunk, chunks_per_record, source_chunks,
      any_chunk, std::vector<int>::iterator(selected_columns_iterator));
  FinalPositionSetupFromBufferToInterface(
      last_chunk_leftover_size, current_offset_point, target_positions,
      any_position, selected_columns_iterator);
}

void DMACrossbarSetup::InitialDataSetupFromInterfaceToBuffer(
    const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, const int& current_position_shift,
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    std::vector<int>::iterator& selected_columns_iterator) {
  for (int initial_chunk_count = 0; initial_chunk_count < chunks_per_record - 1;
       initial_chunk_count++) {
    const int current_chunk =
        current_cycle_step_initial_chunk + initial_chunk_count;

    InitialChunkSetupFromInterfaceToBuffer(
        current_offset_point, source_chunks, current_chunk,
        std::vector<int>::iterator(selected_columns_iterator));
    InitialPositionSetupFromInterfaceToBuffer(
        current_position_shift, target_positions, selected_columns_iterator);
  }
}

void DMACrossbarSetup::FinalDataSetupFromInterfaceToBuffer(
    const int& used_leftover_chunks_count,
    const int& current_cycle_step_initial_chunk,
    const int& last_chunk_leftover_size, const int& chunks_per_record,
    std::queue<int>& source_chunks, const int& any_chunk,
    std::queue<int>& target_positions, const int& any_position,
    std::vector<int>::iterator& selected_columns_iterator) {
  // Chunks and positions are set in the same fashion from interface to buffer
  const int initial_any_chunk_sequence_size =
      query_acceleration_constants::kDatapathWidth -
      (used_leftover_chunks_count %
       query_acceleration_constants::kDatapathWidth) -
      last_chunk_leftover_size;

  int initial_valid_chunk_sequence_size = 0;
  int last_any_chunk_sequence_size = 0;
  int last_valid_chunk_sequence_size = 0;

  if (initial_any_chunk_sequence_size < 0) {
    initial_valid_chunk_sequence_size =
        last_chunk_leftover_size + initial_any_chunk_sequence_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
    last_valid_chunk_sequence_size = std::abs(initial_any_chunk_sequence_size);
  } else {
    initial_valid_chunk_sequence_size = last_chunk_leftover_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size - initial_any_chunk_sequence_size;
    last_valid_chunk_sequence_size = 0;
  }

  for (int initial_any_chunk_index = 0;
       initial_any_chunk_index < initial_any_chunk_sequence_size;
       initial_any_chunk_index++) {
    source_chunks.push(any_chunk);
    target_positions.push(any_position);
  }
  for (int initial_valid_chunk_index = 0;
       initial_valid_chunk_index < initial_valid_chunk_sequence_size;
       initial_valid_chunk_index++) {
    // source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record -
    //                   1);
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
    const int current_target_position =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size + last_valid_chunk_sequence_size +
        initial_valid_chunk_index;
    target_positions.push(current_target_position);
  }
  for (int last_any_chunk_index = 0;
       last_any_chunk_index < last_any_chunk_sequence_size;
       last_any_chunk_index++) {
    source_chunks.push(any_chunk);
    target_positions.push(any_position);
  }
  for (int last_valid_chunk_index = 0;
       last_valid_chunk_index < last_valid_chunk_sequence_size;
       last_valid_chunk_index++) {
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
    // source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record);
    const int current_target_position =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size + last_valid_chunk_index;
    target_positions.push(current_target_position);
  }
}

void DMACrossbarSetup::FinalPositionSetupFromBufferToInterface(
    const int& last_chunk_leftover_size, const int& current_offset_point,
    std::queue<int>& target_positions, const int& any_position,
    std::vector<int>::iterator& selected_columns_iterator) {
  for (int empty_initial_chunk_position = 0;
       empty_initial_chunk_position <
       query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
       empty_initial_chunk_position++) {
    target_positions.push(any_position);
  }
  for (int left_over_chunk_position = 0;
       left_over_chunk_position < last_chunk_leftover_size;
       left_over_chunk_position++) {
    auto next_element_it =
        std::next(selected_columns_iterator,
                  last_chunk_leftover_size - left_over_chunk_position);
    const int current_position =
        (query_acceleration_constants::kDatapathWidth - *next_element_it) % 16;
    if (current_position < 0) {
      target_positions.push(query_acceleration_constants::kDatapathWidth +
                            current_position);
    } else {
      target_positions.push(current_position);
    }
  }
  std::advance(selected_columns_iterator, last_chunk_leftover_size);
}

void DMACrossbarSetup::FinalChunkSetupFromBufferToInterface(
    const int& used_leftover_chunks_count, const int& last_chunk_leftover_size,
    const int& current_cycle_step_initial_chunk, const int& chunks_per_record,
    std::queue<int>& source_chunks, const int& any_chunk,
    std::vector<int>::iterator& selected_columns_iterator) {
  const int initial_any_chunk_sequence_size =
      query_acceleration_constants::kDatapathWidth -
      (used_leftover_chunks_count %
       query_acceleration_constants::kDatapathWidth) -
      last_chunk_leftover_size;

  int initial_valid_chunk_sequence_size = 0;
  int last_any_chunk_sequence_size = 0;
  int last_valid_chunk_sequence_size = 0;

  if (initial_any_chunk_sequence_size < 0) {
    initial_valid_chunk_sequence_size =
        last_chunk_leftover_size + initial_any_chunk_sequence_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
    last_valid_chunk_sequence_size = std::abs(initial_any_chunk_sequence_size);
  } else {
    initial_valid_chunk_sequence_size = last_chunk_leftover_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size - initial_any_chunk_sequence_size;
    last_valid_chunk_sequence_size = 0;
  }

  for (int initial_any_chunk_index = 0;
       initial_any_chunk_index < initial_any_chunk_sequence_size;
       initial_any_chunk_index++) {
    source_chunks.push(any_chunk);
  }
  for (int initial_valid_chunk_index = 0;
       initial_valid_chunk_index < initial_valid_chunk_sequence_size;
       initial_valid_chunk_index++) {
    // source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record -
    //                   1);
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
  }
  for (int last_any_chunk_index = 0;
       last_any_chunk_index < last_any_chunk_sequence_size;
       last_any_chunk_index++) {
    source_chunks.push(any_chunk);
  }
  for (int last_valid_chunk_index = 0;
       last_valid_chunk_index < last_valid_chunk_sequence_size;
       last_valid_chunk_index++) {
    // source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record);
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
  }
}

void DMACrossbarSetup::InitialPositionSetupFromBufferToInterface(
    const int& current_offset_point, std::queue<int>& target_positions,
    std::vector<int>::iterator& selected_columns_iterator) {
  // for (int forward_chunk_position = current_offset_point;
  //     forward_chunk_position < query_acceleration_constants::kDatapathWidth;
  //     forward_chunk_position++) {
  //  //target_positions.push(forward_chunk_position);
  //  auto next_element_it = std::next(
  //      selected_columns_iterator,
  //      forward_chunk_position);
  //  target_positions.push(*next_element_it % 16);
  //}
  // for (int current_chunk_position = 0;
  //     current_chunk_position < current_offset_point;
  //     current_chunk_position++) {
  //  //target_positions.push(current_chunk_position);
  //  auto next_element_it = std::next(
  //      selected_columns_iterator,
  //      current_chunk_position);
  //  target_positions.push(*next_element_it % 16);
  //}
  for (int i = query_acceleration_constants::kDatapathWidth; i > 0; i--) {
    auto next_element_it = std::next(selected_columns_iterator, i);
    const int current_position =
        (query_acceleration_constants::kDatapathWidth - *next_element_it) % 16;
    if (current_position < 0) {
      target_positions.push(query_acceleration_constants::kDatapathWidth +
                            current_position);
    } else {
      target_positions.push(current_position);
    }
  }
  std::advance(selected_columns_iterator, 16);
  // for (int i = 0; i < query_acceleration_constants::kDatapathWidth; i++) {
  //  target_positions.push(*selected_columns_iterator % 16);
  //  selected_columns_iterator++;
  //}
}

void DMACrossbarSetup::InitialChunkSetupFromBufferToInterface(
    const int& current_offset_point, std::queue<int>& source_chunks,
    const int& current_chunk,
    std::vector<int>::iterator& selected_columns_iterator) {
  // Beginning of the chunk until current_offset_point
  // for (int current_position = 0; current_position < current_offset_point;
  //     current_position++) {
  //  source_chunks.push(current_chunk);
  //}
  //// Rest of the chunk
  // for (int forward_chunk_position = current_offset_point;
  //     forward_chunk_position < query_acceleration_constants::kDatapathWidth;
  //     forward_chunk_position++) {
  //  source_chunks.push(current_chunk + 1);
  //}
  for (int i = 0; i < query_acceleration_constants::kDatapathWidth; i++) {
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
  }
}

void DMACrossbarSetup::InitialPositionSetupFromInterfaceToBuffer(
    const int& current_position_shift, std::queue<int>& target_positions,
    std::vector<int>::iterator& selected_columns_iterator) {
  // for (int current_position = current_position_shift;
  //     current_position < query_acceleration_constants::kDatapathWidth;
  //     current_position++) {
  //  target_positions.push(current_position);
  //}
  // for (int forward_chunk_position = 0;
  //     forward_chunk_position < current_position_shift;
  //     forward_chunk_position++) {
  //  target_positions.push(forward_chunk_position);
  //}
  for (int i = 0; i < query_acceleration_constants::kDatapathWidth; i++) {
    target_positions.push(*selected_columns_iterator % 16);
    selected_columns_iterator++;
  }
}

void DMACrossbarSetup::InitialChunkSetupFromInterfaceToBuffer(
    const int& current_offset_point, std::queue<int>& source_chunks,
    const int& current_chunk,
    std::vector<int>::iterator& selected_columns_iterator) {
  //// Beginning of the chunk until current_offset_point
  // for (int current_position = 0; current_position < current_offset_point;
  //     current_position++) {
  //  source_chunks.push(current_chunk);
  //}
  //// Rest of the chunk (current_position_shift + current_offset_point =
  //// datapath_width)
  // for (int forward_chunk_position = current_offset_point;
  //     forward_chunk_position < query_acceleration_constants::kDatapathWidth;
  //     forward_chunk_position++) {
  //  source_chunks.push(current_chunk + 1);
  //}
  for (int i = 0; i < query_acceleration_constants::kDatapathWidth; i++) {
    source_chunks.push(*selected_columns_iterator / 16);
    selected_columns_iterator++;
  }
}

void DMACrossbarSetup::SetCrossbarSetupDataForStream(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    DMASetupData& stream_setup_data) {
  for (int current_buffer_chunk = 0;
       current_buffer_chunk < query_acceleration_constants::kDatapathLength;
       current_buffer_chunk++) {
    DMACrossbarSetupData current_chunk_data;
    for (int current_data_input = 0;
         current_data_input < query_acceleration_constants::kDatapathWidth;
         current_data_input++) {
      current_chunk_data.chunk_selection[current_data_input] =
          source_chunks.front();
      source_chunks.pop();
      current_chunk_data.position_selection[current_data_input] =
          target_positions.front();
      target_positions.pop();
    }
    stream_setup_data.crossbar_setup_data.push_back(current_chunk_data);
  }
  for (const auto& thing : stream_setup_data.crossbar_setup_data) {
    std::cout << std::endl;
    for (const auto& thing2 : thing.chunk_selection) {
      std::cout << thing2 << " ";
    }
  }
}

void DMACrossbarSetup::AddBubbleChunkAndPositionData(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& chunks_per_record, const int& any_chunk,
    const int& any_position) {
  const int next_power_of_two =
      StreamParameterCalculator::FindNextPowerOfTwo(chunks_per_record);
  if (next_power_of_two != chunks_per_record) {
    for (int bubble_element_id = 0;
         bubble_element_id < (next_power_of_two - chunks_per_record) *
                                 query_acceleration_constants::kDatapathWidth;
         bubble_element_id++) {
      source_chunks.push(any_chunk);
      target_positions.push(any_position);
    }
  }
}