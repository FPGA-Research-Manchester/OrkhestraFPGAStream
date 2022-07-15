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

#include "csv_reader_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to read given CSV data files row by row either as string data or
 * 32bit integer data.
 */
class CSVReader : public CSVReaderInterface {
 private:
  /**
   * @brief Check if the given memory block is large enough for the data in the
   * given file.
   * @param filename Name of the file where the table data is stored.
   * @param memory_device Pointer to the memory block where the data will be
   * written to.
   * @return Boolean noting if the data fits.
   */
  static auto IsMemoryLargeEnough(
      const std::string& filename,
      MemoryBlockInterface* memory_device) -> bool;
  /**
   * @brief Write data to given memory pointer with the given offset.
   * @param data 32bit integer data to be written to memory.
   * @param address Virtual pointer to the memory block.
   * @param offset Offset where the data should be written to.
   */
  static void WriteDataToMemory(const std::vector<uint32_t>& data,
                                volatile uint32_t* address, int offset);

 public:
  ~CSVReader() override = default;

  /**
   * @brief Read CSV file and store the string data vector.
   * @param filename File where the data is.
   * @param separator Separator character.
   * @param rows_already_read Offset of where to start reading data from.
   * @return String data vector
   */
  auto ReadTableData(const std::string& filename, char separator,
                     int& rows_already_read)
      -> std::vector<std::vector<std::string>> override;
  /**
   * @brief Write data directly from filesystem to given memory block.
   * @param filename Data file.
   * @param separator CSV data separator character.
   * @param column_defs_vector Column types and sizes.
   * @param memory_device Memory block pointer.
   * @return How many rows of data were read.
   */
  auto WriteTableFromFileToMemory(
      const std::string& filename, char separator,
      const std::vector<std::pair<ColumnDataType, int>>& column_defs_vector,
      MemoryBlockInterface* memory_device)
      -> int override;
};

}  // namespace orkhestrafs::dbmstodspi