#pragma once

#include <cstdint>
#include <string>
#include <vector>

class DataArraysConverter {
 public:
  static void AddIntegerDataFromStringData(
      const std::vector<std::vector<std::string>>& string_data,
      std::vector<uint32_t>& integer_data);
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& string_data,
      const std::vector<int>& data_type_sizes);

  static void ConvertStringValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector);
  static void ConvertIntegerValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector);
  static void ConvertStringValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  static void ConvertIntegerValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);

 private:
  static auto ConvertHexStringToString(const std::string& hex) -> std::string;
  static auto Convert32CharStringToAscii(const std::string& input_string)
      -> std::vector<int>;
};