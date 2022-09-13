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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "logger.hpp"
#include "types_converter.hpp"
#include "util.hpp"

using orkhestrafs::dbmstodspi::CSVReader;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::util::IsValidFile;

auto CSVReader::IsMemoryLargeEnough(const std::string& filename,
                                    MemoryBlockInterface* memory_device)
    -> bool {
  try {
    auto data_size = std::filesystem::file_size(filename);
    auto memory_size = memory_device->GetSize();

    Log(LogLevel::kTrace, "MEMORY SIZE = " + std::to_string(memory_size));
    Log(LogLevel::kDebug, "READING " + filename + " WITH THE SIZE OF " +
                              std::to_string(data_size));

    return data_size <= memory_size;
  } catch (std::filesystem::filesystem_error& ex) {
    throw std::runtime_error(ex);
  }
}

void CSVReader::WriteDataToMemory(const std::vector<uint32_t>& data,
                                  volatile uint32_t* address, int offset) {
  std::copy(data.begin(), data.end(), &address[offset]);
  /*for (int i = 0; i < data.size(); i++) {
    address[i + offset] = data[i];
  }*/
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
            intermediate[intermediate.size() - 1] == '\r') {
          intermediate.erase(intermediate.size() - 1);
        }
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
    MemoryBlockInterface* memory_device) -> int {
  if (!IsValidFile(filename)) {
    throw std::runtime_error(filename + " not found!");
  }
  if (!IsMemoryLargeEnough(filename, memory_device)) {
    throw std::runtime_error(filename + " is too big!");
  }

  int row_counter = 0;
  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }
  volatile uint32_t* input = memory_device->GetVirtualAddress();

  if (column_defs_vector.size() == 1 &&
      column_defs_vector.front().first == ColumnDataType::kPixel) {
    // Need to read binary instead
    char* buffer;
    std::ifstream file(filename,
                       std::ios::in | std::ios::binary | std::ios::ate);
    // TODO: Add check that it is divisible by 16.
    long byte_size = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer = new char[byte_size];
    file.read(buffer, byte_size);
    file.close();

    for (int i = 0; i < byte_size; i += 4) {
      input[(i / 4)] = buffer[i + 3] | (buffer[i + 2] << 8) |
                            (buffer[i + 1] << 16) | (buffer[i] << 24);
    }

    /*for (int i = 0; i < byte_size; i += 16) {
      input[(i / 16) * 4] = buffer[i + 3] | (buffer[i + 2] << 8) |
                     (buffer[i + 1] << 16) | (buffer[i] << 24);
      input[(i / 16) * 4 + 1] = buffer[i + 7] | (buffer[i + 6] << 8) |
                       (buffer[i + 5] << 16) | (buffer[i+4] << 24);
      input[(i / 16) * 4 + 2] = buffer[i + 11] | (buffer[i + 10] << 8) |
                       (buffer[i + 9] << 16) | (buffer[i+8] << 24);
      input[(i / 16) * 4 + 3] = buffer[i + 15] | (buffer[i + 14] << 8) |
                       (buffer[i + 13] << 16) | (buffer[i+12] << 24);
    }*/

    return byte_size / record_size / 4; // 4 channels in one word
  }

  std::vector<uint32_t> integer_data;
  std::vector<std::string> tokens;

  std::ifstream filestream(filename);
  std::string line;
  std::string token_string;

  while (std::getline(filestream, line)) {
    tokens.clear();
    std::stringstream linestream(line);
    while (getline(linestream, token_string, separator)) {
      if (!token_string.empty() &&
          token_string[token_string.size() - 1] == '\r') {
        token_string.erase(token_string.size() - 1);
      }

      tokens.push_back(std::move(token_string));
    }

    integer_data.clear();
    TypesConverter::ConvertRecordStringToIntegers(tokens, column_defs_vector,
                                                  integer_data);
    std::copy(integer_data.begin(), integer_data.end(),
              &input[row_counter * record_size]);
    // WriteDataToMemory(integer_data, input, row_counter * record_size);
    row_counter++;
  }

  return row_counter;
}