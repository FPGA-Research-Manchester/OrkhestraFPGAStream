#include "table_manager.hpp"

#include <iostream>
#include <stdexcept>

auto TableManager::GetRecordSizeFromTable(const TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void TableManager::ReadOutputDataFromMemoryBlock(
    const std::unique_ptr<MemoryBlockInterface>& output_device,
    TableData& resulting_table, const int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSizeFromTable(resulting_table)));
}

void TableManager::WriteInputDataToMemoryBlock(
    const std::unique_ptr<MemoryBlockInterface>& input_device,
    const TableData& input_table) {
  std::cout << "Table size: " << input_table.table_data_vector.size() * 4 / 1000
            << "[KB]" << std::endl;
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
    std::string table_name,
    const std::unique_ptr<MemoryBlockInterface>& input_device,
    const TableData& input_table) {
  auto output_table = input_table;
  const int table_size = static_cast<int>(input_table.table_data_vector.size() /
                                          GetRecordSizeFromTable(input_table));

  TableManager::ReadOutputDataFromMemoryBlock(input_device, output_table,
                                              table_size);

  std::cout << std::endl
            << "Table " << table_name << " with " << table_size << " rows"
            << std::endl;
  std::cout << std::hex << "Address: "
            << reinterpret_cast<uintptr_t>(input_device->GetPhysicalAddress())
            << std::dec << std::endl;
  DataManager::PrintTableData(output_table);
}

void TableManager::ReadInputTables(
    std::vector<StreamDataParameters>& input_stream_parameters,
    DataManager& data_manager,
    const std::vector<std::string>& stream_data_file_names,
    const std::vector<int>& stream_id_vector,
    std::vector<std::unique_ptr<MemoryBlockInterface>>&
        allocated_memory_blocks) {
  if (stream_data_file_names.size() != stream_id_vector.size()) {
    throw std::runtime_error("The amount of stream IDs given is not correct!");
  }
  for (int stream_index = 0; stream_index < stream_data_file_names.size();
       stream_index++) {
    auto current_table =
        data_manager.ParseDataFromCSV(stream_data_file_names[stream_index]);

    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      WriteInputDataToMemoryBlock(allocated_memory_blocks[stream_index],
                                  current_table);
      /*PrintWrittenData(stream_data_file_names[stream_index],
                       allocated_memory_blocks[stream_index], current_table);*/
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }

    StreamDataParameters current_stream_parameters = {
        stream_id_vector[stream_index], GetRecordSizeFromTable(current_table),
        static_cast<int>(current_table.table_data_vector.size() /
                         GetRecordSizeFromTable(current_table)),
        physical_address_ptr};

    input_stream_parameters.push_back(current_stream_parameters);
  }
}

void TableManager::ReadExpectedTables(
    std::vector<StreamDataParameters>& output_stream_parameters,
    DataManager& data_manager,
    const std::vector<std::string>& stream_data_file_names,
    const std::vector<int>& stream_id_vector,
    std::vector<std::unique_ptr<MemoryBlockInterface>>& allocated_memory_blocks,
    std::vector<TableData>& output_tables) {
  if (stream_data_file_names.size() != stream_id_vector.size()) {
    throw std::runtime_error("The amount of stream IDs given is not correct!");
  }
  for (int stream_index = 0; stream_index < stream_data_file_names.size();
       stream_index++) {
    auto current_table =
        data_manager.ParseDataFromCSV(stream_data_file_names[stream_index]);

    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }
    StreamDataParameters current_stream_parameters = {
        stream_id_vector[stream_index], GetRecordSizeFromTable(current_table),
        0, physical_address_ptr};

    output_stream_parameters.push_back(current_stream_parameters);

    output_tables[stream_id_vector[stream_index]] = current_table;
  }
}

void TableManager::ReadResultTables(
    const std::vector<StreamDataParameters>& output_stream_parameters,
    std::vector<TableData>& output_tables,
    const std::vector<int>& result_record_counts,
    std::vector<std::unique_ptr<MemoryBlockInterface>>&
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