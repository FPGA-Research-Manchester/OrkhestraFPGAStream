#include "table_manager.hpp"

#include <sstream>
#include <stdexcept>

#include "logger.hpp"
#include "query_scheduling_data.hpp"

using namespace dbmstodspi::query_managing;

using dbmstodspi::logger::Log;
using dbmstodspi::logger::LogLevel;
using dbmstodspi::logger::ShouldLog;
using dbmstodspi::query_managing::query_scheduling_data::kIOStreamParamDefs;

auto TableManager::GetRecordSizeFromTable(
    const data_managing::TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void TableManager::ReadOutputDataFromMemoryBlock(
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& output_device,
    data_managing::TableData& resulting_table, const int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSizeFromTable(resulting_table)));
}

void TableManager::WriteInputDataToMemoryBlock(
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& input_device,
    const data_managing::TableData& input_table) {
  PrintDataSize(input_table);
  if (input_table.table_data_vector.size() * 4 > input_device->GetSize()) {
    throw std::runtime_error(
        "Not enough memory in the allocated memory block!");
  }
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }
}

// Debug method
void TableManager::PrintWrittenData(
    const std::string& table_name,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& input_device,
    const data_managing::TableData& input_table) {
  auto log_level = dbmstodspi::logger::LogLevel::kTrace;
  if (dbmstodspi::logger::ShouldLog(log_level)) {
    std::stringstream ss;
    auto output_table = input_table;
    const int table_size =
        static_cast<int>(input_table.table_data_vector.size() /
                         GetRecordSizeFromTable(input_table));

    TableManager::ReadOutputDataFromMemoryBlock(input_device, output_table,
                                                table_size);

    ss << "Table " << table_name << std::hex << "Address: "
       << reinterpret_cast<uintptr_t>(input_device->GetPhysicalAddress())
       << std::dec;
    dbmstodspi::logger::Log(log_level, ss.str());
    data_managing::DataManager::PrintTableData(output_table);
  }
}

void TableManager::ReadInputTables(
    std::vector<fpga_managing::StreamDataParameters>& input_stream_parameters,
    data_managing::DataManager& data_manager,
    const std::vector<std::string>& stream_data_file_names,
    const std::vector<int>& stream_id_vector,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks,
    const std::vector<std::vector<int>>& stream_specification) {
  if (stream_data_file_names.size() != stream_id_vector.size()) {
    throw std::runtime_error("The amount of stream IDs given is not correct!");
  }
  for (int stream_index = 0; stream_index < stream_data_file_names.size();
       stream_index++) {
    Log(LogLevel::kDebug,
        "Reading input: " + stream_data_file_names[stream_index]);
    auto current_table =
        data_manager.ParseDataFromCSV(stream_data_file_names[stream_index]);

    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      WriteInputDataToMemoryBlock(allocated_memory_blocks[stream_index],
                                  current_table);
      PrintWrittenData(stream_data_file_names[stream_index],
                       allocated_memory_blocks[stream_index], current_table);
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }

    const int record_count =
        static_cast<int>(current_table.table_data_vector.size() /
                         GetRecordSizeFromTable(current_table));

    fpga_managing::StreamDataParameters current_stream_parameters = {
        stream_id_vector[stream_index], GetRecordSizeFromTable(current_table),
        record_count, physical_address_ptr,
        stream_specification.at(stream_index *
                                    kIOStreamParamDefs.kStreamParamCount +
                                kIOStreamParamDefs.kProjectionOffset)};

    Log(LogLevel::kTrace,
        "RECORD_SIZE = " +
            std::to_string(GetRecordSizeFromTable(current_table)) +
            "[integers]");
    Log(LogLevel::kDebug, "RECORD_COUNT = " + std::to_string(record_count));

    input_stream_parameters.push_back(current_stream_parameters);
  }
}

void TableManager::ReadExpectedTables(
    std::vector<fpga_managing::StreamDataParameters>& output_stream_parameters,
    data_managing::DataManager& data_manager,
    const std::vector<std::string>& stream_data_file_names,
    const std::vector<int>& stream_id_vector,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks,
    std::vector<data_managing::TableData>& output_tables,
    const std::vector<std::vector<int>>& stream_specification) {
  if (stream_data_file_names.size() != stream_id_vector.size()) {
    throw std::runtime_error("The amount of stream IDs given is not correct!");
  }
  for (int stream_index = 0; stream_index < stream_data_file_names.size();
       stream_index++) {
    Log(LogLevel::kDebug,
        "Reading output: " + stream_data_file_names[stream_index]);
    auto current_table =
        data_manager.ParseDataFromCSV(stream_data_file_names[stream_index]);

    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }
    fpga_managing::StreamDataParameters current_stream_parameters = {
        stream_id_vector[stream_index],
        GetRecordSizeFromTable(current_table),
        0,
        physical_address_ptr,
        stream_specification.at(stream_index *
                                    kIOStreamParamDefs.kStreamParamCount +
                                kIOStreamParamDefs.kProjectionOffset),
        stream_specification.at(stream_index *
                                    kIOStreamParamDefs.kStreamParamCount +
                                kIOStreamParamDefs.kChunkCountOffset).at(0)};

    PrintDataSize(current_table);
    Log(LogLevel::kTrace,
        "RECORD_SIZE = " +
            std::to_string(GetRecordSizeFromTable(current_table)) +
            "[integers]");

    output_stream_parameters.push_back(current_stream_parameters);

    output_tables[stream_id_vector[stream_index]] = current_table;
  }
}

void TableManager::ReadResultTables(
    const std::vector<fpga_managing::StreamDataParameters>&
        output_stream_parameters,
    std::vector<data_managing::TableData>& output_tables,
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

void TableManager::PrintDataSize(const data_managing::TableData& data_table) {
  Log(LogLevel::kDebug,
      "Table size: " +
          std::to_string(data_table.table_data_vector.size() * 4 / 1000) +
          "[KB]");
}