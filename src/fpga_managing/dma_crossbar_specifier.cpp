#include "dma_crossbar_specifier.hpp"

#include <math.h>

#include <iostream>
#include <map>
#include <set>
#include <utility>

#include "query_acceleration_constants.hpp"

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
  for (int i = 0; i < record_specification.size(); i++) {
    if (record_specification[i] != -1) {
      if (chunk_count.find(record_specification[i] /
                           query_acceleration_constants::kDatapathWidth) ==
          chunk_count.end()) {
        chunk_count[record_specification[i] /
                    query_acceleration_constants::kDatapathWidth] = 1;
      } else {
        chunk_count[record_specification[i] /
                    query_acceleration_constants::kDatapathWidth]++;
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

void DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
    const int record_size, std::vector<int>& record_specification,
    const int records_per_ddr_burst, int& chunks_per_record) {}

void DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
    const int record_size, std::vector<int>& record_specification,
    int& records_per_ddr_burst) {}

void DMACrossbarSpecifier::ResolveOutputClashesMultiChannel(
    const int record_size, std::vector<int>& record_specification,
    const int records_per_ddr_burst, int& chunks_per_record) {}

void DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
    const int record_size, const std::vector<int> record_specification,
    int& records_per_ddr_burst) {}

auto DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
    const int record_size, const std::vector<int> record_specification,
    const int records_per_ddr_burst, const int chunks_per_record)
    -> std::vector<int> {
  return std::vector<int>();
}

auto DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
    const int record_size, const std::vector<int> record_specification,
    const int records_per_ddr_burst) -> std::vector<int> {
  return std::vector<int>();
}
