#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include "table_data.hpp"

class DataManager {
 public:
  explicit DataManager(const std::string& config_filename)
      : data_type_sizes_(DataManager::GetDataConfiguration(config_filename)){};
  auto ParseDataFromCSV(const std::string& filename) -> TableData;
  static void PrintTableData(const TableData& table_data);

 private:
  std::map<std::string, double> data_type_sizes_;
  static auto GetDataConfiguration(const std::string& config_filename)
      -> std::map<std::string, double>;
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& string_data,
      const std::vector<std::pair<std::string, int>>& data_types_vector);
  static void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data);
};