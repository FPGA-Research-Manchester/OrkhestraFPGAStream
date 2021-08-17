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

#include "csv_reader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "types_converter.hpp"

using namespace dbmstodspi::data_managing;

auto CSVReader::CheckDataFits(
    const std::string& filename,
    const std::unique_ptr<MemoryBlockInterface>& memory_device) -> bool {
  return true;
}

void CSVReader::WriteDataToMemory(const std::vector<uint32_t>& data,
                                  volatile uint32_t* address, int offset) {
  for (int i = 0; i < data.size(); i++) {
    address[i + offset] = data[i];
  }
}

auto CSVReader::ReadTableData(const std::string& filename, char separator,
                              int& rows_already_read)
    -> std::vector<std::vector<std::string>> {
  std::ifstream big_file(filename);
  std::vector<std::vector<std::string>> data;
  std::string line;
  int max_rows = 2000000;
  int row_counter = 0;
  while (std::getline(big_file, line)) {
    row_counter++;
    if (row_counter > rows_already_read) {
      std::vector<std::string> tokens;
      std::stringstream linestream(line);
      std::string intermediate;
      while (getline(linestream, intermediate, separator)) {
        if (!intermediate.empty() &&
            intermediate[intermediate.size() - 1] == '\r')
          intermediate.erase(intermediate.size() - 1);
        tokens.push_back(intermediate);
      }
      data.push_back(tokens);
    }
    if (row_counter >= rows_already_read + max_rows) {
      rows_already_read = row_counter;
      return data;
    }
  }
  rows_already_read = row_counter;
  return data;
}

auto CSVReader::WriteTableFromFileToMemory(
    const std::string& filename, char separator,
    const std::vector<std::pair<ColumnDataType, int>>& column_defs_vector,
    const std::unique_ptr<MemoryBlockInterface>& memory_device) -> int {
  CheckDataFits(filename, memory_device);

  std::vector<uint32_t> integer_data;
  std::vector<std::string> tokens;

  std::ifstream filestream(filename);
  std::string line;
  std::string token_string;

  int row_counter = 0;
  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }
  volatile uint32_t* input = memory_device->GetVirtualAddress();

  while (std::getline(filestream, line)) {
    tokens.clear();
    std::stringstream linestream(line);
    while (getline(linestream, token_string, separator)) {
      if (!token_string.empty() &&
          token_string[token_string.size() - 1] == '\r') {
        token_string.erase(token_string.size() - 1);
      }

      tokens.push_back(token_string);
    }

    integer_data.clear();
    TypesConverter::ConvertRecordStringToIntegers(tokens, column_defs_vector,
                                                  integer_data);
    WriteDataToMemory(integer_data, input, row_counter * record_size);
    row_counter++;
  }
  return row_counter;
}