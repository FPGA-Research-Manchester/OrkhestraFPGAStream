#include "types_converter.hpp"

#include <cstdlib>
#include <map>
#include <sstream>

void TypesConverter::AddIntegerDataFromStringData(
    const std::vector<std::vector<std::string>>& string_data,
    std::vector<uint32_t>& integer_data,
    std::vector<std::pair<std::string, int>> data_types_vector) {
  std::map<std::string,
           void (*)(const std::string&, std::vector<uint32_t>&, int)>
      conversion_functions;
  conversion_functions.insert(std::make_pair(
      "integer", TypesConverter::ConvertIntegerValuesToIntegerData));
  conversion_functions.insert(std::make_pair(
      "varchar", TypesConverter::ConvertStringValuesToIntegerData));
  conversion_functions.insert(std::make_pair(
      "null", TypesConverter::ConvertNullValuesToIntegerData));

  for (const auto &row : string_data) {
    for (int column = 0; column < row.size(); column++) {
      conversion_functions[data_types_vector[column].first](
          row[column], integer_data, data_types_vector[column].second);
    }
  }
}

void TypesConverter::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& resulting_string_data,
    const std::vector<std::pair<std::string, int>>& data_types_vector) {
  std::map<std::string,
           void (*)(const std::vector<uint32_t>&, std::vector<std::string>&)>
      conversion_functions;
  conversion_functions.insert(
      std::make_pair("integer", TypesConverter::ConvertIntegerValuesToString));
  conversion_functions.insert(
      std::make_pair("varchar", TypesConverter::ConvertStringValuesToString));
  conversion_functions.insert(
      std::make_pair("null", TypesConverter::ConvertNullValuesToString));

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
  if (input_string.length() > output_size * 4) {
    throw std::runtime_error(
        (input_string + " is longer than " + std::to_string(output_size * 4))
            .c_str());
  }
  std::vector<int> integer_values(output_size, 0);
  for (int i = 0; i < input_string.length(); i++) {
    integer_values[i / 4] += int(input_string[i]) << (3 - (i % 4)) * 8;
  }
  return integer_values;
}
