#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class TypesConverter {
 public:
  static void AddIntegerDataFromStringData(
      const std::vector<std::vector<std::string>>& string_data,
      std::vector<uint32_t>& integer_data,
      std::vector<std::pair<std::string, int>> data_types_vector);
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& resulting_string_data,
      const std::vector<std::pair<std::string, int>>& data_types_vector);

  static void ConvertStringValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  static void ConvertIntegerValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  static void ConvertNullValuesToIntegerData(const std::string& input,
                                             std::vector<uint32_t>& data_vector,
                                             int output_size);
  static void ConvertDecimalValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  static void ConvertDateValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  static void ConvertStringValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  static void ConvertIntegerValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  static void ConvertNullValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  static void ConvertDecimalValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  static void ConvertDateValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);

 private:
  static auto ConvertHexStringToString(const std::string& hex) -> std::string;
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;
};