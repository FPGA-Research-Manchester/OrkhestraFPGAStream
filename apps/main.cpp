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

void ConvToInt(const std::string& pStr, StringAsciiValue& pVal) {
  for (int i = 0; i < pStr.length(); i++) {
    pVal.values[i / 4] += int(pStr[i]) << (3 - (i % 4)) * 8;
  }
}

void FillDataArray(std::vector<int>& dbData, rapidcsv::Document* dbDataFile) {
  int current_local_data_address = 0;
  for (int row_number = 0; row_number < dbDataFile->GetRowCount();
       row_number++) {
    for (int col_number = 0; col_number < 4; col_number++) {
      switch (col_number) {
        case 0:
        case 3:
          dbData[current_local_data_address++] =
              dbDataFile->GetCell<int>(col_number, row_number);
          break;
        case 1:
        case 2:
          auto ascii_value = dbDataFile->GetCell<StringAsciiValue>(
              col_number, row_number, ConvToInt);
          for (int value : ascii_value.values) {
            dbData[current_local_data_address++] = value;
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

  // Create the controller memory area //Also hardcode the address to 0xA0000000
  // we're going to use baremetal
  int* volatile memory_pointer = new int[2097152];
  for (int i = 0; i < 2097152; i++) {
    memory_pointer[i] = -1;
  }

  Setup setup_configurer;
  Setup::SetupQueryAcceleration(memory_pointer, db_data, record_size,
                                doc.GetRowCount());

  delete[] memory_pointer;
  return 0;
}
