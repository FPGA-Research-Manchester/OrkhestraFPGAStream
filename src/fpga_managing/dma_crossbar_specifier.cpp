#include "dma_crossbar_specifier.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

using namespace dbmstodspi::fpga_managing;

auto DMACrossbarSpecifier::IsInputClashing(
    const std::vector<int>& record_specification) -> bool {
  int integers_processed = 0;
  while (integers_processed != record_specification.size()) {
    int chunk_end_index = 0;
    if (integers_processed + query_acceleration_constants::kDatapathWidth <
        record_specification.size()) {
      chunk_end_index =
          integers_processed + query_acceleration_constants::kDatapathWidth;
    } else {
      chunk_end_index = record_specification.size();
    }
    std::set<int> current_chunk_set(
        record_specification.begin() + integers_processed,
        record_specification.begin() + chunk_end_index);
    integers_processed = chunk_end_index;
    std::map<int, int> positional_map;
    for (const auto& selected_location : current_chunk_set) {
      auto result = positional_map.insert(std::pair<int, int>(
          selected_location % query_acceleration_constants::kDatapathWidth, 1));
      if (!result.second) {
        return true;
      }
    }
  }
  return false;
}

auto DMACrossbarSpecifier::IsOutputClashing(
    const std::vector<int>& record_specification) -> bool {
  if (record_specification.size() <=
      query_acceleration_constants::kDatapathWidth) {
    return false;
  }
  for (int positions_processed = 0;
       positions_processed < query_acceleration_constants::kDatapathWidth;
       positions_processed++) {
    std::vector<int> current_position_locations;
    current_position_locations.reserve(std::ceil(
        (record_specification.size() - positions_processed) /
        static_cast<double>(query_acceleration_constants::kDatapathWidth)));
    for (int chunk_id = 0;
         chunk_id <
         std::ceil(
             (record_specification.size() - positions_processed) /
             static_cast<double>(query_acceleration_constants::kDatapathWidth));
         chunk_id++) {
      current_position_locations.push_back(
          record_specification[positions_processed +
                               query_acceleration_constants::kDatapathWidth *
                                   chunk_id]);
    }
    std::map<int, int> positional_map;
    for (const auto& selected_location : current_position_locations) {
      if (selected_location != -1) {
        auto result = positional_map.insert(std::pair<int, int>(
            selected_location / query_acceleration_constants::kDatapathWidth,
            1));
        if (!result.second) {
          return true;
        }
      }
    }
  }
  return false;
}

auto DMACrossbarSpecifier::IsOutputOverwritingData(
    const std::vector<int>& record_specification) -> bool {
  std::map<int, int> chunk_count;
  for (int i : record_specification) {
    if (i != -1) {
      if (chunk_count.find(i / query_acceleration_constants::kDatapathWidth) ==
          chunk_count.end()) {
        chunk_count[i / query_acceleration_constants::kDatapathWidth] = 1;
      } else {
        chunk_count[i / query_acceleration_constants::kDatapathWidth]++;
      }
    }
  }
  for (const auto& [chunk, count] : chunk_count) {
    if (count > query_acceleration_constants::kDatapathWidth) {
      return true;
    }
  }
  return false;
}

// Remove code duplication between these two methods
void DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
    const int /*record_size*/, std::vector<int>& record_specification,
    const int /*records_per_ddr_burst*/, int& /*chunks_per_record*/) {
  while (IsInputClashing(record_specification)) {
    record_specification = {-1};
  }
}
void DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
    const int /*record_size*/, std::vector<int>& record_specification,
    int& /*records_per_ddr_burst*/) {
  while (IsInputClashing(record_specification)) {
    record_specification = {-1};
  }
}

void DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
    int /*record_size*/, std::vector<int>& record_specification,
    int& /*records_per_ddr_burst*/) {
  if (IsOutputOverwritingData(record_specification)) {
    throw std::runtime_error("Unresolvable input!");
  }
  while (IsOutputClashing(record_specification)) {
    record_specification = {-1};
  }
}

auto DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
    const int record_size, const std::vector<int>& record_specification,
    const int records_per_ddr_burst) -> const std::vector<int> {
  return ExtendSpecification(
      records_per_ddr_burst, record_specification, record_size, -1,
      query_acceleration_constants::kDdrBurstSize / records_per_ddr_burst);
}

auto DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
    const int record_size, const std::vector<int>& record_specification,
    const int records_per_ddr_burst, const int chunks_per_record)
    -> const std::vector<int> {
  return ExtendSpecification(
      records_per_ddr_burst, record_specification, record_size, -1,
      StreamParameterCalculator::FindNextPowerOfTwo(chunks_per_record) *
          query_acceleration_constants::kDatapathWidth);
}

// In this method we are using -2 to denote a backspace kind of symbol to say
// where does the record end. This is to keep track of the garbage data between
// records.
auto DMACrossbarSpecifier::ExtendOutputSpecification(
    const std::vector<int>& record_specification,
    const int records_per_ddr_burst, const int chunks_per_record)
    -> const std::vector<int> {
  return ExtendSpecification(
      records_per_ddr_burst, record_specification,
      chunks_per_record * query_acceleration_constants::kDatapathWidth, -2,
      query_acceleration_constants::kDdrBurstSize / records_per_ddr_burst);
}

auto DMACrossbarSpecifier::ExtendSpecification(
    const int& records_per_ddr_burst,
    const std::vector<int>& record_specification, const int& record_size,
    const int post_record_junk_data, const int junk_data_end_point)
    -> const std::vector<int> {
  std::vector<int> extended_specification;
  std::vector<int> chunk_specfication;
  for (int record_index = 0; record_index < records_per_ddr_burst;
       record_index++) {
    InsertValidRecordData(record_specification, record_size * record_index,
                          chunk_specfication, extended_specification);
    InsertJunkDataAfterRecord(record_specification.size(), junk_data_end_point,
                              post_record_junk_data, chunk_specfication,
                              extended_specification);
  }
  if (!IsSpecificationValid(extended_specification)) {
    throw std::runtime_error(
        "Incorrect parameters created a specification which doesn't fit!");
  }
  return extended_specification;
}

void DMACrossbarSpecifier::InsertValidRecordData(
    const std::vector<int>& record_specification, const int start_point,
    std::vector<int>& chunk_specfication,
    std::vector<int>& extended_specification) {
  for (const auto& selection : record_specification) {
    if (selection != -1) {
      const int extended_selection = selection + start_point;
      const int selection_original_chunk =
          extended_selection / query_acceleration_constants::kDatapathWidth;
      // Position is reversed such that the first integer is at the LSB side.
      const int selection_position =
          query_acceleration_constants::kDatapathWidth - 1 -
          (extended_selection % query_acceleration_constants::kDatapathWidth);

      chunk_specfication.push_back(
          (query_acceleration_constants::kDatapathWidth *
               (selection_original_chunk) +
           selection_position));
    } else {
      chunk_specfication.push_back(selection);
    }
    InsertChunkIfFull(chunk_specfication, extended_specification);
  }
}

void DMACrossbarSpecifier::InsertJunkDataAfterRecord(
    const int start_point, const int end_point, const int junk_data,
    std::vector<int>& chunk_specfication,
    std::vector<int>& extended_specification) {
  for (int junk_data_index = start_point; junk_data_index < end_point;
       junk_data_index++) {
    chunk_specfication.push_back(junk_data);
    InsertChunkIfFull(chunk_specfication, extended_specification);
  }
}

void DMACrossbarSpecifier::InsertChunkIfFull(
    std::vector<int>& chunk_specfication,
    std::vector<int>& extended_specification) {
  if (chunk_specfication.size() ==
      query_acceleration_constants::kDatapathWidth) {
    std::reverse(chunk_specfication.begin(), chunk_specfication.end());
    extended_specification.insert(std::end(extended_specification),
                                  std::begin(chunk_specfication),
                                  std::end(chunk_specfication));
    chunk_specfication.clear();
  }
}

auto DMACrossbarSpecifier::IsSpecificationValid(
    const std::vector<int>& record_specification) -> bool {
  return record_specification.size() <=
             query_acceleration_constants::kDdrBurstSize &&
         *std::max_element(record_specification.begin(),
                           record_specification.end()) <
             query_acceleration_constants::kDdrBurstSize;
}