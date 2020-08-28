#pragma once
#include <rapidcsv.h>

#include <vector>
#include <string>

class CSVReader {
 public:
  static void AddCSVRows(const std::string& file_name,
                         std::vector<std::vector<std::string>>& read_rows);
};