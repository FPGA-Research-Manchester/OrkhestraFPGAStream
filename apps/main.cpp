#include <rapidcsv.h>

#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "setup.hpp"

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | integer               |
 make   | character varying(32) |
 model  | character varying(32) |
 price  | integer               |
*/

struct StringAsciiValue {
  std::vector<int> values;
  StringAsciiValue() : values(32 / 4, 0) {}
};

void ConvToInt(const std::string& p_str, StringAsciiValue& p_val) {
  for (int i = 0; i < p_str.length(); i++) {
    p_val.values[i / 4] += int(p_str[i]) << (3 - (i % 4)) * 8;
  }
}

void FillDataArray(std::vector<uint32_t>& db_data,
                   rapidcsv::Document* db_data_file) {
  int current_local_data_address = 0;
  for (int row_number = 0; row_number < db_data_file->GetRowCount();
       row_number++) {
    for (int col_number = 0; col_number < 4; col_number++) {
      switch (col_number) {
        case 0:
        case 3:
          db_data[current_local_data_address++] =
              db_data_file->GetCell<int>(col_number, row_number);
          break;
        case 1:
        case 2:
          auto ascii_value = db_data_file->GetCell<StringAsciiValue>(
              col_number, row_number, ConvToInt);
          for (int value : ascii_value.values) {
            db_data[current_local_data_address++] = value;
          }
          break;
      }
    }
  }
}

// auto main() -> int {
//  // Figure out some legit way to get this data type information. For all
//  // streams. Would be nice to have this info in structs or sth like that to
//  // capture dataType
//  std::vector<int> data_type_sizes{1, 8, 8, 1};
//  int record_size =
//      std::accumulate(data_type_sizes.begin(), data_type_sizes.end(), 0);
//
//  // The data probably doesn't come in a csv but it'll do for now
//  rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
//
//  int data_size = doc.GetRowCount() * record_size;
//  // Create contiguous data array
//  std::vector<uint32_t> input_memory_area(data_size);
//  FillDataArray(input_memory_area, &doc);
//
//  std::vector<uint32_t> output_memory_area(data_size);
//  std::vector<uint32_t> module_configuration_memory_area(262144, -1);
//
//  Setup::SetupQueryAcceleration(
//      module_configuration_memory_area.data(), input_memory_area.data(),
//      output_memory_area.data(), record_size, doc.GetRowCount());
//
//  return 0;
//}

std::vector<int> Convert32CharStringToAscii(const std::string input_string) {
  std::vector<int> integer_values(32 / 4, 0);
  for (int i = 0; i < input_string.length(); i++) {
    integer_values[i / 4] += int(input_string[i]) << (3 - (i % 4)) * 8;
  }
  return integer_values;
}

void AddStringValuesToData(const std::string input,
                           std::vector<uint32_t>& data_vector) {
  for (auto value : Convert32CharStringToAscii(input)) {
    data_vector.push_back(value);
  }
}

void AddIntegerValuesToData(const std::string input,
                            std::vector<uint32_t>& data_vector) {
  data_vector.push_back(std::stoi(input));
}

std::string ConvertHexStringToString(std::string hex) {
  std::string resulting_string;
  for (int i = 0; i < hex.length(); i += 2) {
    std::string byte = hex.substr(i, 2);
    char chr = (char)(int)strtol(byte.c_str(), nullptr, 16);
    resulting_string.push_back(chr);
  }
  return resulting_string;
}

void ConvertStringValuesToString(const std::vector<uint32_t> input_value,
                                 std::vector<std::string>& string_vector) {
  std::stringstream ss;
  for (auto value : input_value) {
    ss << std::hex << value;
  }
  string_vector.push_back(ConvertHexStringToString(ss.str()));
}

void ConvertIntegerValuesToString(const std::vector<uint32_t> input_value,
                                  std::vector<std::string>& string_vector) {
  string_vector.push_back(std::to_string(input_value[0]));
}

auto main() -> int {
  std::vector<int> data_type_sizes{1, 8, 8, 1};
  int record_size =
      std::accumulate(data_type_sizes.begin(), data_type_sizes.end(), 0);

  rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
  int record_count = doc.GetRowCount();
  // int record_count = 2;
  std::vector<std::vector<std::string>> db_data;
  for (int row_number = 0; row_number < record_count; row_number++) {
    db_data.push_back(doc.GetRow<std::string>(row_number));
  }

  std::vector<uint32_t> input_memory_area;
  std::vector<void (*)(const std::string, std::vector<uint32_t>&)>
      conversion_functions = {AddIntegerValuesToData, AddStringValuesToData,
                              AddStringValuesToData, AddIntegerValuesToData};

  for (auto row : db_data) {
    for (int column = 0; column < row.size(); column++) {
      conversion_functions[column](row[column], input_memory_area);
    }
  }
  int data_size = doc.GetRowCount() * record_size;
  std::vector<uint32_t> output_memory_area(data_size);
  std::vector<uint32_t> module_configuration_memory_area(262144, -1);

  Setup::SetupQueryAcceleration(
      module_configuration_memory_area.data(), input_memory_area.data(),
      output_memory_area.data(), record_size, doc.GetRowCount());

  return 0;

  /*for (auto row : db_data) {
    for (auto element : row) {
      std::cout << element << " ";
    }
    std::cout << std::endl;
  }*/

  /*std::vector<std::vector<std::string>> input_data = {
      {"1", "Mitsubishi", "Galant", "54708"}, {"2", "Porsche", "911", "54289"}};

  for (auto row : db_data) {
    for (auto element : row) {
      std::cout << element << " ";
    }
    std::cout << std::endl;
  }*/

  /*std::vector<std::vector<std::string>> output_data;
  std::vector<void (*)(const std::vector<uint32_t>, std::vector<std::string>&)>
      back_conversion_functions = {
          ConvertIntegerValuesToString, ConvertStringValuesToString,
          ConvertStringValuesToString, ConvertIntegerValuesToString};
  std::vector<uint32_t> current_element;
  std::vector<std::string> current_output_row;
  int current_column_index = 0;
  for (int element_id = 0; element_id < int_data.size(); element_id++) {
    current_element.push_back(int_data[element_id]);
    if (current_element.size() == data_type_sizes[current_column_index]) {
      back_conversion_functions[current_column_index](current_element,
                                                      current_output_row);
      current_element.clear();
      if ((current_column_index + 1) == data_type_sizes.size()) {
        output_data.push_back(current_output_row);
        current_output_row.clear();
      }
      current_column_index =
          (current_column_index + 1) % data_type_sizes.size();
    }
  }

  for (auto row : output_data) {
    for (auto element : row) {
      std::cout << element << " ";
    }
    std::cout << std::endl;
  }*/

  /*std::vector<RowData> input_data = {{1, "Mitsubishi", "Galant", 54708},
                                     {2, "Porsche", "911", 54289}};
  std::cout << Convert32CharStringToAscii(input_data[0].make)[0] << std::endl
            << Convert32CharStringToAscii(input_data[0].make)[1] << std::endl
            << Convert32CharStringToAscii(input_data[0].make)[2] << std::endl
            << Convert32CharStringToAscii(input_data[0].make)[3] << std::endl;
  std::cout << input_data[0].model.substr(0, 3) << std::endl;
  std::cout << input_data[1].make.substr(0, 3) << std::endl;
  std::cout << input_data[1].model.substr(0, 3) << std::endl;*/
}