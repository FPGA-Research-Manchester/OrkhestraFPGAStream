#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

class DataManager {
 public:
  static auto GetDataConfiguration(std::string config_filename)
      -> std::map<std::string, double>;
  static void AddStringDataFromCSV(
      const std::string& file_name,
      std::vector<std::vector<std::string>>& resulting_strings);
  static void AddIntegerDataFromStringData(
      const std::vector<std::vector<std::string>>& string_data,
      std::vector<uint32_t>& integer_data);
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& string_data,
      const std::vector<int>& data_type_sizes);
  static void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data);
};