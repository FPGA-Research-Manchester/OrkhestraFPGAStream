#pragma once

#include <cstdint>
#include <string>
#include <vector>

class DataManager {
 public:
  static void AddStringDataFromCSV(
      std::string file_name,
      std::vector<std::vector<std::string>>& resulting_strings);
  static void AddIntegerDataFromStringData(
      std::vector<std::vector<std::string>> string_data,
      std::vector<uint32_t>& integer_data);
  static void AddStringDataFromIntegerData(
      std::vector<uint32_t> integer_data,
      std::vector<std::vector<std::string>>& string_data, std::vector<int>);
  static void PrintStringData(
      std::vector<std::vector<std::string>> string_data);
};