#include <rapidcsv.h>

#include <vector>
#include<numeric>

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
  std::vector<int> data_type_sizes{1,8,8,1};
  int record_size = std::accumulate(data_type_sizes.begin(), data_type_sizes.end(), 0);

  // The data probably doesn't come in a csv but it'll do for now
  rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));

  int data_size = doc.GetRowCount() * record_size;
  // Create contiguous data array
  std::vector<int> db_data(data_size);
  FillDataArray(db_data, &doc);

  std::vector<int> output_memory_address(data_size);
  std::vector<int> memory_pointer(2097152, -1);

  Setup::SetupQueryAcceleration(reinterpret_cast<unsigned int>(&memory_pointer[0]), db_data,
                                output_memory_address.data(),
                                record_size, doc.GetRowCount());

  return 0;
}