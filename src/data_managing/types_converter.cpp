#include "types_converter.hpp"

#include <cstdlib>
#include <sstream>

void DataArraysConverter::AddIntegerDataFromStringData(
    const std::vector<std::vector<std::string>>& string_data,
    std::vector<uint32_t>& integer_data) {
  std::vector<void (*)(const std::string&, std::vector<uint32_t>&)>
      conversion_functions = {
          DataArraysConverter::ConvertIntegerValuesToIntegerData,
          DataArraysConverter::ConvertStringValuesToIntegerData,
          DataArraysConverter::ConvertStringValuesToIntegerData,
          DataArraysConverter::ConvertIntegerValuesToIntegerData};

  for (auto row : string_data) {
    for (int column = 0; column < row.size(); column++) {
      conversion_functions[column](row[column], integer_data);
    }
  }
}

void DataArraysConverter::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& string_data,
    const std::vector<int>& data_type_sizes) {
  std::vector<void (*)(const std::vector<uint32_t>&, std::vector<std::string>&)>
      conversion_functions = {
          DataArraysConverter::ConvertIntegerValuesToString,
          DataArraysConverter::ConvertStringValuesToString,
          DataArraysConverter::ConvertStringValuesToString,
          DataArraysConverter::ConvertIntegerValuesToString};

  std::vector<uint32_t> current_element;
  std::vector<std::string> current_output_row;
  int current_column_index = 0;
  for (unsigned int element_id : integer_data) {
    current_element.push_back(element_id);
    if (current_element.size() == data_type_sizes[current_column_index]) {
      conversion_functions[current_column_index](current_element,
                                                      current_output_row);
      current_element.clear();
      if ((current_column_index + 1) == data_type_sizes.size()) {
        string_data.push_back(current_output_row);
        current_output_row.clear();
      }
      current_column_index =
          (current_column_index + 1) % data_type_sizes.size();
    }
  }
}

void DataArraysConverter::ConvertStringValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector) {
  for (auto value : Convert32CharStringToAscii(input)) {
    data_vector.push_back(value);
  }
}

void DataArraysConverter::ConvertIntegerValuesToIntegerData(
    const std::string& input, std::vector<uint32_t>& data_vector) {
  data_vector.push_back(std::stoi(input));
}

void DataArraysConverter::ConvertStringValuesToString(
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

void DataArraysConverter::ConvertIntegerValuesToString(
    const std::vector<uint32_t>& input_value,
    std::vector<std::string>& string_vector) {
  string_vector.push_back(std::to_string(input_value[0]));
}

auto DataArraysConverter::ConvertHexStringToString(const std::string& hex)
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

auto DataArraysConverter::Convert32CharStringToAscii(
    const std::string& input_string) -> std::vector<int> {
  std::vector<int> integer_values(32 / 4, 0);
  for (int i = 0; i < input_string.length(); i++) {
    integer_values[i / 4] += int(input_string[i]) << (3 - (i % 4)) * 8;
  }
  return integer_values;
}
