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
   * @return read_rows Main data of the CSV file.
   */
  static auto ReadTableData(const std::string& filename)
      -> std::vector<std::vector<std::string>>;
};

}  // namespace dbmstodspi::data_managing