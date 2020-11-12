#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include "table_data.hpp"

class DataManager {
 public:
  static auto ParseDataFromCSV(std::string filename,
                               std::map<std::string, double> data_type_sizes)
      -> TableData;
  static auto GetDataConfiguration(std::string config_filename)
      -> std::map<std::string, double>;
  /*static void AddStringDataFromCSV(
      const std::string& file_name,
      std::vector<std::vector<std::string>>& resulting_strings);*/
  //static void AddIntegerDataFromStringData(
  //    const std::vector<std::vector<std::string>>& string_data,
  //    std::vector<uint32_t>& integer_data,
  //    std::vector<std::pair<std::string, int>> data_types_vector);
  static void PrintTableData(const TableData table_data);
 private:
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& string_data,
      const std::vector<std::pair<std::string, int>> data_types_vector);
  static void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data);
};