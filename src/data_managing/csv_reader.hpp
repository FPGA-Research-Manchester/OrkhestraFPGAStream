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
#include <rapidcsv.h>

#include <map>
#include <string>
#include <vector>

#include "table_data.hpp"

namespace dbmstodspi::data_managing {

/**
 * @brief Class which uses rapidcsv to read a CSV file for input and output
 * data.
 *
 * rapidcsv source is available at https://github.com/d99kris/rapidcsv.
 */
class CSVReader {
 public:
  /**
   * @brief Read the given CSV file and return row data.
   * @param filename Path to the CSV file
   * @param separator Separator character
   * @return read_rows Main data of the CSV file.
   */
  static auto ReadTableData(const std::string& filename, char separator)
      -> std::vector<std::vector<std::string>>;
};

}  // namespace dbmstodspi::data_managing