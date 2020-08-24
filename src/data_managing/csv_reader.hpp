#pragma once
#include <rapidcsv.h>

#include <vector>
#include <string>

class CSVReader {
 public:
  static void AddCSVRows(std::string file_name,
                        std::vector<std::vector<std::string>>& output_vector);
};