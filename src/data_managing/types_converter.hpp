#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "table_data.hpp"

using dbmstodspi::data_managing::table_data::ColumnDataType;

namespace dbmstodspi::data_managing {

/**
 * @brief Class to convert different types of data to string and integer
 * formats.
 */
class TypesConverter {
 public:
  /**
   * @brief Add integer typed data to the output vector given input data in
   * string format.
   * @param string_data Input data vector.
   * @param integer_data Output data vector.
   * @param data_types_vector Column data types.
   */
  static void AddIntegerDataFromStringData(
      const std::vector<std::vector<std::string>>& string_data,
      std::vector<uint32_t>& integer_data,
      std::vector<std::pair<ColumnDataType, int>> data_types_vector);
  /**
   * @brief Add string typed data to the output vector given input data in
   * integer format.
   * @param integer_data Input data vector.
   * @param resulting_string_data Output data vector.
   * @param data_types_vector Column data types.
   */
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& resulting_string_data,
      const std::vector<std::pair<ColumnDataType, int>>& data_types_vector);

  /**
   * @brief Convert string table element to integers.
   * @param input String element.
   * @param data_vector Output integer vector.
   * @param output_size String size.
   */
  static void ConvertStringValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  /**
   * @brief Convert integer table element to integers.
   * @param input String input representing an integer.
   * @param data_vector Output integer vector.
   * @param output_size How many integers should be added to the output.
   */
  static void ConvertIntegerValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  /**
   * @brief Convert null table element to integers.
   * @param input String input representing a NULL value.
   * @param data_vector Output integer vector.
   * @param output_size How many integers should be added to the output.
   */
  static void ConvertNullValuesToIntegerData(const std::string& input,
                                             std::vector<uint32_t>& data_vector,
                                             int output_size);
  /**
   * @brief Convert decimal table element to integers.
   * @param input String input representing a decimal value.
   * @param data_vector Output integer vector.
   * @param output_size How many integers should be added to the output.
   */
  static void ConvertDecimalValuesToIntegerData(
      const std::string& input, std::vector<uint32_t>& data_vector,
      int output_size);
  /**
   * @brief Convert date table element to integers.
   * @param input String input representing a date value.
   * @param data_vector Output integer vector.
   * @param output_size How many integers should be added to the output
   */
  static void ConvertDateValuesToIntegerData(const std::string& input,
                                             std::vector<uint32_t>& data_vector,
                                             int output_size);
  /**
   * @brief Convert a vector of integers to string format.
   * @param input_value Integer vector representing a string value.
   * @param string_vector Output string vector.
   */
  static void ConvertStringValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);

  /**
   * @brief Convert a vector of integers to string format.
   * @param input_value Integer vector representing an integer value.
   * @param string_vector Output string vector.
   */
  static void ConvertIntegerValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  /**
   * @brief Convert a vector of integers to string format.
   * @param input_value Integer vector representing a NULL value.
   * @param string_vector Output string vector.
   */
  static void ConvertNullValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  /**
   * @brief Convert a vector of integers to string format.
   * @param input_value Integer vector representing a decimal value.
   * @param string_vector Output string vector.
   */
  static void ConvertDecimalValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);
  /**
   * @brief Convert a vector of integers to string format.
   * @param input_value Integer vector representing a date value.
   * @param string_vector Output string vector.
   */
  static void ConvertDateValuesToString(
      const std::vector<uint32_t>& input_value,
      std::vector<std::string>& string_vector);

 private:
  static void ConvertDataToIntegers(ColumnDataType data_type, const std::string& input_string,
                                    std::vector<uint32_t>& converted_data_vector,
                                    int string_size);
  static void ConvertDataToString(ColumnDataType data_type,
                                  const std::vector<uint32_t>& input_integer_data,
                                  std::vector<std::string>& converted_data_vector);

  /**
   * @brief Helper method to convert strings with hexadecimal values to strings.
   * @param hex Input string with hex values which can be converted to a string
   * with ASCII characters.
   * @return Converted Ascii string.
   */
  static auto ConvertHexStringToString(const std::string& hex) -> std::string;
  /**
   * @brief Helper method to convert strings to ASCII integer values.
   * @param input_string String to be converted to integers.
   * @param output_size How many integers can be used.
   * @return Vector of integers representing the ASCII integer value of the
   * input string.
   */
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;
};

}  // namespace dbmstodspi::data_managing