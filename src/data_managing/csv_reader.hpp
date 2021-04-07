#pragma once
#include <rapidcsv.h>

#include <map>
#include <string>
#include <vector>

#include "table_data.hpp"

/**
 * @brief Class which uses rapidcsv to read a CSV file for input and output
 * data.
 *
 * rapidcsv source is available at https://github.com/d99kris/rapidcsv.
 */
class CSVReader {
 public:
  /**
   * @brief Read the given CSV file and write the header and row data into the
   * given string vectors.
   * @param filename Path to the CSV file
   * @param header_row First row of the CSV file which tells the data types and
   * sizes of the columns.
   * @param read_rows Main data of the CSV file.
   */
  static void ReadTableData(const std::string& filename,
                            std::vector<std::string>& header_row,
                            std::vector<std::vector<std::string>>& read_rows);
};