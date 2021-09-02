/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "types_converter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

using easydspi::core_interfaces::table_data::ColumnDataType;
using easydspi::dbmstodspi::TypesConverter;

void TypesConverter::AddIntegerDataFromStringData(
    const std::vector<std::vector<std::string>>& string_data,
    std::vector<uint32_t>& integer_data,
    const std::vector<std::pair<ColumnDataType, int>>& data_types_vector) {
  for (const auto& row : string_data) {
    ConvertRecordStringToIntegers(row, data_types_vector, integer_data);
  }
}

void TypesConverter::ConvertRecordStringToIntegers(
    const std::vector<std::string>& row,
    const std::vector<
        std::pair<ColumnDataType, int>>&
        data_types_vector,
    std::vector<uint32_t>& integer_data) {
  for (int column = 0; column < row.size(); column++) {
    ConvertDataToIntegers(data_types_vector[column].first, row[column],
                          integer_data, data_types_vector[column].second);
  }
}

void TypesConverter::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& resulting_string_data,
    const std::vector<std::pair<ColumnDataType, int>>& data_types_vector) {
  std::vector<uint32_t> current_element;
  std::vector<std::string> current_output_row;
  int current_column_index = 0;
  for (unsigned int element_id : integer_data) {
    current_element.push_back(element_id);
    if (current_element.size() ==
        data_types_vector[current_column_index].second) {
      ConvertDataToString(data_types_vector[current_column_index].first,
                          current_element, current_output_row);
      current_element.clear();
      if ((current_column_index + 1) == data_types_vector.size()) {
        resulting_string_data.push_back(current_output_row);
        current_output_row.clear();
      }
      current_column_index =
          (current_column_index + 1) % data_types_vector.size();
    }
  }
}

void TypesConverter::ConvertDataToIntegers(
    ColumnDataType data_type, const std::string& input_string,
    std::vector<uint32_t>& converted_data_vector, int string_size) {
  switch (data_type) {
    case ColumnDataType::kInteger:
      ConvertIntegerValuesToIntegerData(input_string, converted_data_vector,
                                        string_size);
      break;
    case ColumnDataType::kVarchar:
      ConvertStringValuesToIntegerData(input_string, converted_data_vector,
                                       string_size);
      break;
    case ColumnDataType::kNull:
      ConvertNullValuesToIntegerData(input_string, converted_data_vector,
                                     string_size);
      break;
    case ColumnDataType::kDecimal:
      ConvertDecimalValuesToIntegerData(input_string, converted_data_vector,
                                        string_size);
      break;
    case ColumnDataType::kDate:
      ConvertDateValuesToIntegerData(input_string, converted_data_vector,
                                     string_size);
      break;
    default:
      throw std::runtime_error("Incorrect data type given!");
  }
}
void TypesConverter::ConvertDataToString(
    ColumnDataType data_type, const std::vector<uint32_t>& input_integer_data,
    std::vector<std::string>& converted_data_vector) {
  switch (data_type) {
    case ColumnDataType::kInteger:
      ConvertIntegerValuesToString(input_integer_data, converted_data_vector);
      break;
    case ColumnDataType::kVarchar:
      ConvertStringValuesToString(input_integer_data, converted_data_vector);
      break;
    case ColumnDataType::kNull:
      ConvertNullValuesToString(input_integer_data, converted_data_vector);
      break;
    case ColumnDataType::kDecimal:
      ConvertDecimalValuesToString(input_integer_data, converted_data_vector);
      break;
    case ColumnDataType::kDate:
      ConvertDateValuesToString(input_integer_data, converted_data_vector);
      break;
    default:
      throw std::runtime_error("Incorrect data type given!");
  }
}

void TypesConverter::ConvertStringValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector,
    int output_size) {
  // Should throw error when output_size is anything other than 1
  for (auto value : ConvertCharStringToAscii(input, output_size)) {
    data_vector.push_back(value);
  }
}

void TypesConverter::ConvertIntegerValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector,
    int /*output_size*/) {
  data_vector.push_back(std::stoi(input));
}

void TypesConverter::ConvertNullValuesToIntegerData(
    const std::string& /*input*/, std::vector<uint32_t>& data_vector,
    int output_size) {
  for (int i = 0; i < output_size; i++) {
    data_vector.push_back(0);
  }
}

void TypesConverter::ConvertDecimalValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector,
    int /*output_size*/) {
  long long input_value = std::round(std::stod(input) * 100.0);
  data_vector.push_back(static_cast<uint32_t>(input_value >> 32));
  data_vector.push_back(static_cast<uint32_t>(input_value & 0xFFFFFFFF));
}

void TypesConverter::ConvertDateValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector,
    int /*output_size*/) {
  auto input_copy = input;
  input_copy.erase(std::remove(input_copy.begin(), input_copy.end(), '-'),
                   input_copy.end());
  data_vector.push_back(std::stoi(input_copy));
}

void TypesConverter::ConvertStringValuesToString(
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  std::stringstream ss;
  for (auto value : input_value) {
    if (value != 0) {
      ss << std::hex << value;
    }
  }
  string_vector.push_back(ConvertHexStringToString(ss.str()));
}

void TypesConverter::ConvertIntegerValuesToString(
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  string_vector.push_back(std::to_string(input_value[0]));
}

void TypesConverter::ConvertNullValuesToString(
    const std::vector<uint32_t>& /*input_value*/,
    std::vector<std::string>& string_vector) {
  string_vector.emplace_back("");
}

void TypesConverter::ConvertDecimalValuesToString(
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2)
      << static_cast<long long>((static_cast<uint64_t>(input_value[0]) << 32) +
                                input_value[1]) /
             100.0;
  string_vector.push_back(oss.str());
}

void TypesConverter::ConvertDateValuesToString(
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  auto string_value = std::to_string(input_value[0]);
  if (string_value.size() < 8) {
    string_value.insert(0, 8 - string_value.size(), '0');
  }
  string_value.insert(4, "-");
  string_value.insert(7, "-");
  string_vector.push_back(string_value);
}

auto TypesConverter::ConvertHexStringToString(const std::string& hex)
    -> std::string {
  std::string resulting_string;
  for (int i = 0; i < hex.length(); i += 2) {
    std::string byte = hex.substr(i, 2);
    if (byte != "00") {
      char character = std::stoul(byte, nullptr, 16);
      resulting_string.push_back(character);
    }
  }
  return resulting_string;
}

auto TypesConverter::ConvertCharStringToAscii(const std::string& input_string,
                                              int output_size)
    -> std::vector<int> {
  // If the string is surrounded by quotes we need to reduce parsing range
  int length_reduction = 0;
  int start_shift = 0;
  if (input_string.at(0) == '"' && input_string.back() == '"') {
    length_reduction = 2;
    start_shift = 1;
  }
  if ((input_string.length() - length_reduction) > (output_size * 4)) {
    throw std::runtime_error(
        (input_string + " is longer than " + std::to_string(output_size * 4))
            .c_str());
  }
  std::vector<int> integer_values(output_size, 0);
  for (int i = 0; i < input_string.length() - length_reduction; i++) {
    integer_values[i / 4] += int(input_string.at(i + start_shift))
                             << (3 - (i % 4)) * 8;
  }
  return integer_values;
}
