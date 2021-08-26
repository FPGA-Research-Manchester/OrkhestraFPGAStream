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

#include "table_manager.hpp"

#include <sstream>
#include <stdexcept>

#include "logger.hpp"
#include "query_scheduling_data.hpp"
#include "table_data.hpp"

using namespace dbmstodspi::query_managing;

using dbmstodspi::logger::Log;
using dbmstodspi::logger::LogLevel;
using dbmstodspi::logger::ShouldLog;
using dbmstodspi::query_managing::query_scheduling_data::kIOStreamParamDefs;

using dbmstodspi::data_managing::table_data::ColumnDataType;
using dbmstodspi::data_managing::table_data::TableData;

auto TableManager::GetRecordSizeFromTable(const TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void TableManager::ReadOutputDataFromMemoryBlock(
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& output_device,
    TableData& resulting_table, const int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSizeFromTable(resulting_table)));
}

void TableManager::WriteInputDataToMemoryBlock(
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& input_device,
    const TableData& input_table, int previous_record_count) {
  PrintDataSize(input_table);
  if (input_table.table_data_vector.size() * 4 > input_device->GetSize()) {
    throw std::runtime_error(
        "Not enough memory in the allocated memory block!");
  }
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i + (previous_record_count * record_size)] =
        input_table.table_data_vector[i];
  }
}

// Debug method
void TableManager::PrintWrittenData(
    const data_managing::DataManager& data_manager,
    const std::string& table_name,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& input_device,
    const TableData& input_table) {
  auto log_level = dbmstodspi::logger::LogLevel::kTrace;
  if (dbmstodspi::logger::ShouldLog(log_level)) {
    std::stringstream ss;
    auto output_table = input_table;
    const int table_size =
        static_cast<int>(input_table.table_data_vector.size() /
                         GetRecordSizeFromTable(input_table));

    TableManager::ReadOutputDataFromMemoryBlock(input_device, output_table,
                                                table_size);

    ss << "Table " << table_name << std::hex << " address: "
       << reinterpret_cast<uintptr_t>(input_device->GetPhysicalAddress())
       << std::dec;
    dbmstodspi::logger::Log(log_level, ss.str());
    data_manager.PrintTableData(output_table);
  }
}

auto TableManager::WriteDataToMemory(
    const data_managing::DataManager& data_manager,
    const std::vector<std::vector<int>>& stream_specification, int stream_index,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
    const std::string& filename) -> std::pair<int, int> {
  auto column_defs_vector =
      GetColumnDefsVector(data_manager, stream_specification, stream_index);

  auto record_count = data_manager.WriteDataFromCSVToMemory(
      filename, column_defs_vector, memory_device);

  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }

  Log(LogLevel::kTrace,
      "RECORD_SIZE = " + std::to_string(record_size) + "[integers]");
  Log(LogLevel::kDebug, "RECORD_COUNT = " + std::to_string(record_count));

  return {record_size, record_count};
}

auto TableManager::ReadTableFromMemory(
    const data_managing::DataManager& data_manager,
    const std::vector<std::vector<int>>& stream_specification, int stream_index,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
    int row_count) -> TableData {
  auto column_data_types =
      GetColumnDataTypesFromSpecification(stream_specification, stream_index);

  TableData table_data;

  table_data.table_column_label_vector = data_manager.GetHeaderColumnVector(
      column_data_types,
      stream_specification.at(stream_index *
                                  kIOStreamParamDefs.kStreamParamCount +
                              kIOStreamParamDefs.kDataSizesOffset));

  int record_size = 0;
  for (const auto& column_type : table_data.table_column_label_vector) {
    record_size += column_type.second;
  }

  volatile uint32_t* raw_data = memory_device->GetVirtualAddress();
  table_data.table_data_vector =
      std::vector<uint32_t>(raw_data, raw_data + (row_count * record_size));

  return table_data;
}

auto TableManager::ReadTableFromFile(
    const data_managing::DataManager& data_manager,
    const std::vector<std::vector<int>>& stream_specification, int stream_index,
    const std::string& filename) -> TableData {
  Log(LogLevel::kDebug, "Reading file: " + filename);

  auto column_data_types =
      GetColumnDataTypesFromSpecification(stream_specification, stream_index);
  // TODO(Kaspar): Change CSV reading to one buffer only version.
  int read_rows_count = 0;
  return data_manager.ParseDataFromCSV(
      filename, column_data_types,
      stream_specification.at(stream_index *
                                  kIOStreamParamDefs.kStreamParamCount +
                              kIOStreamParamDefs.kDataSizesOffset),
      read_rows_count);
}

auto TableManager::GetColumnDefsVector(
    const data_managing::DataManager& data_manager,
    std::vector<std::vector<int>> node_parameters, int stream_index)
    -> std::vector<std::pair<ColumnDataType, int>> {
  auto column_data_types =
      GetColumnDataTypesFromSpecification(node_parameters, stream_index);
  return data_manager.GetHeaderColumnVector(
      column_data_types,
      node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                         kIOStreamParamDefs.kDataSizesOffset));
}

void TableManager::ReadResultTables(
    const std::vector<fpga_managing::StreamDataParameters>&
        output_stream_parameters,
    std::vector<TableData>& output_tables,
    const std::array<
        int, fpga_managing::query_acceleration_constants::kMaxIOStreamCount>&
        result_record_counts,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks) {
  for (int stream_index = 0; stream_index < allocated_memory_blocks.size();
       stream_index++) {
    if (output_stream_parameters.at(stream_index).physical_address) {
      TableManager::ReadOutputDataFromMemoryBlock(
          allocated_memory_blocks[stream_index],
          output_tables[output_stream_parameters.at(stream_index).stream_id],
          result_record_counts[output_stream_parameters.at(stream_index)
                                   .stream_id]);
    }
  }
}

void TableManager::PrintDataSize(const TableData& data_table) {
  Log(LogLevel::kDebug,
      "Table size: " +
          std::to_string(data_table.table_data_vector.size() * 4 / 1000) +
          "[KB]");
}

auto TableManager::GetColumnDataTypesFromSpecification(
    const std::vector<std::vector<int>>& stream_specification, int stream_index)
    -> std::vector<ColumnDataType> {
  std::vector<ColumnDataType> column_data_types;
  for (const auto& type_int_value : stream_specification.at(
           stream_index * kIOStreamParamDefs.kStreamParamCount +
           kIOStreamParamDefs.kDataTypesOffset)) {
    column_data_types.push_back(static_cast<ColumnDataType>(type_int_value));
  }
  return column_data_types;
}

void TableManager::WriteResultTableFile(
    const data_managing::DataManager& data_manager, const TableData& data_table,
    const std::string& filename) {
  data_manager.WriteTableData(data_table, filename);
}