#pragma once
#include <rapidcsv.h>

#include <string>
#include <vector>
#include <map>

#include "table_data.hpp"

class CSVReader {
 public:
  //static void AddCSVRows(const std::string& file_name,
  //                       std::vector<std::vector<std::string>>& read_rows);
  static void ReadTableData(const std::string& filename,
                            std::vector<std::string>& header_row,
                            std::vector<std::vector<std::string>>& read_rows);
};