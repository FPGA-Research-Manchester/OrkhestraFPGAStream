#include <rapidcsv.h>

#include <Setup.hpp>
#include <vector>

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | bigint                |
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

void FillDataArray(std::vector<int>& db_data,
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

auto main() -> int {
  // Figure out some legit way to get this data type information. For all
  // streams. Would be nice to have this info in structs or sth like that to
  // capture dataType
  std::vector<int> data_type_sizes;
  data_type_sizes.push_back(1);
  data_type_sizes.push_back(8);
  data_type_sizes.push_back(8);
  data_type_sizes.push_back(1);
  // TODO(Kaspar): Check lambda functions performance later here
  int record_size = 0;
  for (int row_size : data_type_sizes) {
    record_size += row_size;
  }

  // The data probably doesn't come in a csv but it'll do for now
  rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
  // Create contiguous data array
  std::vector<int> db_data(doc.GetRowCount() * record_size);
  FillDataArray(db_data, &doc);

  int* volatile output_memory_address =
      static_cast<int*>(malloc(doc.GetRowCount() * record_size));

  // Create the controller memory area 
  int* volatile memory_pointer = new int[2097152];
  for (int i = 0; i < 2097152; i++) {
    memory_pointer[i] = -1;
  }

  //int* volatile memory_pointer = reinterpret_cast<int*>(0xA0000000);

  Setup::SetupQueryAcceleration(memory_pointer, db_data,
                                output_memory_address, record_size,
                                doc.GetRowCount());

  free(output_memory_address);

  return 0;
}
