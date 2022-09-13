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

#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "memory_block_interface.hpp"

namespace orkhestrafs::core_interfaces::table_data {

/**
 * @brief Data types parsable by the library.
 */
enum class ColumnDataType {
  kInteger,
  kVarchar,
  kNull,
  kDecimal,
  kDate,
  kPixel
};

/**
 * @brief String names for the supported data type enums.
 */
const std::map<std::string, ColumnDataType> kDataTypeNames = {
    {"integer", ColumnDataType::kInteger},
    {"varchar", ColumnDataType::kVarchar},
    {"null", ColumnDataType::kNull},
    {"decimal", ColumnDataType::kDecimal},
    {"date", ColumnDataType::kDate},
    {"pixel", ColumnDataType::kPixel}};

/**
 * @brief Struct to hold the information about the table data.
 */
struct TableData {
  /// Vector to hold data about column data types and sizes.
  std::vector<std::pair<ColumnDataType, int>> table_column_label_vector;
  /// Vector to hold the table data in integer format.
  std::vector<uint32_t> table_data_vector;

  auto operator==(const TableData& comparable_table) const -> bool {
    return table_column_label_vector ==
               comparable_table.table_column_label_vector &&
           table_data_vector == comparable_table.table_data_vector;
  }
};

/**
 * @brief Struct to hold information about a sorted sequence
 */
// struct SortedSequence {
//  int start_position;
//  int length;
//  // TODO(Kaspar): add sorted by column
//
//  SortedSequence(int start_position, int length)
//      : start_position(start_position), length(length){};
//};

/**
 * @brief Struct to hold information about a database table.
 */
struct TableMetadata {
  int record_size;  // words
  int record_count;
  // 1. Begin of first set of sequences
  // 2. End of the first set of sequences
  // 3. Size of the sequences
  // 4. How many sequences
  // ... Repeat
  std::vector<int> sorted_status;

  auto operator==(const TableMetadata& rhs) const -> bool {
    return record_size == rhs.record_size && record_count == rhs.record_count &&
           sorted_status == rhs.sorted_status;
  }

  auto operator!=(const TableMetadata& rhs) const -> bool {
    return record_size != rhs.record_size || record_count != rhs.record_count ||
           sorted_status != rhs.sorted_status;
  }
};

}  // namespace orkhestrafs::core_interfaces::table_data