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
 private:
  //static void ProcessChunk(char* buffer, int char_count, char separator,
  //                  std::vector<std::vector<std::string>>& data,
  //                  std::vector<std::string>& current_row,
  //                  std::string& current_element);
  static auto ReadBigFile(const std::string& filename, char separator, int& rows_already_read)
      -> std::vector<std::vector<std::string>>;
 public:
  /**
   * @brief Read the given CSV file and return row data.
   * @param filename Path to the CSV file
   * @param separator Separator character
   * @return read_rows Main data of the CSV file.
   */
  static auto ReadTableData(const std::string& filename, char separator,
                            int& rows_already_read)
      -> std::vector<std::vector<std::string>>;
};

}  // namespace dbmstodspi::data_managing