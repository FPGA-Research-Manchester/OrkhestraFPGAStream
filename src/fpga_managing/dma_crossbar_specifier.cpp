#include "dma_crossbar_specifier.hpp"

#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

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

// Remove code duplication between these two methods
void DMACrossbarSpecifier::ResolveInputClashesMultiChannel(
    const int record_size, std::vector<int>& record_specification,
    const int records_per_ddr_burst, int& chunks_per_record) {
  while (IsInputClashing(record_specification)) {
    record_specification = {-1};
  }
}
void DMACrossbarSpecifier::ResolveInputClashesSingleChannel(
    const int record_size, std::vector<int>& record_specification,
    int& records_per_ddr_burst) {
  while (IsInputClashing(record_specification)) {
    record_specification = {-1};
  }
}

void DMACrossbarSpecifier::ResolveOutputClashesSingleChannel(
    const int record_size, std::vector<int>& record_specification,
    int& records_per_ddr_burst) {
  if (IsOutputOverwritingData(record_specification)) {
    throw std::runtime_error("Unresolvable input!");
  }
  while (IsOutputClashing(record_specification)) {
    record_specification = {-1};
  }
}

// Can't throw any errors. Input only
auto DMACrossbarSpecifier::ExtendSpecificationMultiChannel(
    const int record_size, const std::vector<int> record_specification,
    const int records_per_ddr_burst, const int chunks_per_record)
    -> std::vector<int> {
  std::vector<int> extended_specification;
  const int next_power_of_two =
      StreamParameterCalculator::FindNextPowerOfTwo(chunks_per_record);

  for (int record_index = 0; record_index < records_per_ddr_burst;
       record_index++) {
    for (const auto& selection : record_specification) {
      if (selection != -1) {
        extended_specification.push_back(selection +
                                         (record_size * record_index));
      } else {
        extended_specification.push_back(selection);
      }
    }
    for (int junk_data_index = record_specification.size();
         junk_data_index <
         next_power_of_two * query_acceleration_constants::kDatapathWidth;
         junk_data_index++) {
      extended_specification.push_back(-1);
    }
  }

  std::cout << "multi" << std::endl;
  for (const auto& thing : extended_specification) {
    std::cout << thing << ",";
  }
  std::cout << std::endl;

  return extended_specification;
}
auto DMACrossbarSpecifier::ExtendSpecificationSingleChannel(
    const int record_size, const std::vector<int> record_specification,
    const int records_per_ddr_burst) -> std::vector<int> {
  std::vector<int> extended_specification;

  for (int record_index = 0; record_index < records_per_ddr_burst;
       record_index++) {
    for (const auto& selection : record_specification) {
      if (selection != -1) {
        extended_specification.push_back(selection +
                                         (record_size * record_index));
      } else {
        extended_specification.push_back(selection);
      }
    }
    for (int junk_data_index = record_specification.size();
         junk_data_index < query_acceleration_constants::kDdrBurstSize/records_per_ddr_burst;
         junk_data_index++) {
      extended_specification.push_back(-1);
    }
  }

  std::cout << "single"<< std::endl;
  for (const auto& thing : extended_specification) {
    std::cout << thing << ",";
  }
  std::cout << std::endl;

  return extended_specification;
}

// For output instead of padding with -1 you just increment the thing. I want
// here something from the 27th integer

// Add two more methods to change -1 to valid numbers - can throw errors