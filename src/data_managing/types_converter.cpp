#include "types_converter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace dbmstodspi::data_managing;

void TypesConverter::AddIntegerDataFromStringData(
    const std::vector<std::vector<std::string>>& string_data,
    std::vector<uint32_t>& integer_data,
    std::vector<std::pair<ColumnDataType, int>> data_types_vector) {
  std::map<ColumnDataType,
           void (*)(const std::string&, std::vector<uint32_t>&, int)>
      conversion_functions;
  conversion_functions.insert(
      std::make_pair(ColumnDataType::kInteger,
                     TypesConverter::ConvertIntegerValuesToIntegerData));
  conversion_functions.insert(
      std::make_pair(ColumnDataType::kVarchar,
                     TypesConverter::ConvertStringValuesToIntegerData));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kNull, TypesConverter::ConvertNullValuesToIntegerData));
  conversion_functions.insert(
      std::make_pair(ColumnDataType::kDecimal,
                     TypesConverter::ConvertDecimalValuesToIntegerData));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kDate, TypesConverter::ConvertDateValuesToIntegerData));

  for (const auto& row : string_data) {
    for (int column = 0; column < row.size(); column++) {
      conversion_functions[data_types_vector[column].first](
          row[column], integer_data, data_types_vector[column].second);
    }
  }
}

void TypesConverter::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& resulting_string_data,
    const std::vector<std::pair<ColumnDataType, int>>& data_types_vector) {
  std::map<ColumnDataType,
           void (*)(const std::vector<uint32_t>&, std::vector<std::string>&)>
      conversion_functions;
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kInteger, TypesConverter::ConvertIntegerValuesToString));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kVarchar, TypesConverter::ConvertStringValuesToString));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kNull, TypesConverter::ConvertNullValuesToString));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kDecimal, TypesConverter::ConvertDecimalValuesToString));
  conversion_functions.insert(std::make_pair(
      ColumnDataType::kDate, TypesConverter::ConvertDateValuesToString));

  std::vector<uint32_t> current_element;
  std::vector<std::string> current_output_row;
  int current_column_index = 0;
  for (unsigned int element_id : integer_data) {
    current_element.push_back(element_id);
    if (current_element.size() ==
        data_types_vector[current_column_index].second) {
      conversion_functions[data_types_vector[current_column_index].first](
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
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  // These values are ignored.
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
