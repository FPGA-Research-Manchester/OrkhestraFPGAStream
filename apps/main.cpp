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
  int currentLocalDataAddress = 0;
  for (int rowNumber = 0; rowNumber < dbDataFile->GetRowCount(); rowNumber++) {
    for (int colNumber = 0; colNumber < 4; colNumber++) {
      switch (colNumber) {
        case 0:
        case 3:
          dbData[currentLocalDataAddress++] =
              dbDataFile->GetCell<int>(colNumber, rowNumber);
          break;
        case 1:
        case 2:
          auto asciiValue = dbDataFile->GetCell<StringAsciiValue>(
              colNumber, rowNumber, ConvToInt);
          for (int value : asciiValue.values) {
            dbData[currentLocalDataAddress++] = value;
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
  std::vector<int> dataTypeSizes;
  dataTypeSizes.push_back(1);
  dataTypeSizes.push_back(8);
  dataTypeSizes.push_back(8);
  dataTypeSizes.push_back(1);
  // TODO: Check lambda functions performance later here
  int recordSize = 0;
  for (int rowSize : dataTypeSizes) {
    recordSize += rowSize;
  }

  // The data probably doesn't come in a csv but it'll do for now
  rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
  // Create contiguous data array
  std::vector<int> dbData(doc.GetRowCount() * recordSize);
  FillDataArray(dbData, &doc);

  // Create the controller memory area //Also hardcode the address to 0xA0000000
  // we're going to use baremetal
  int* volatile memoryPointer = new int[2097152];
  for (int i = 0; i < 2097152; i++) {
    memoryPointer[i] = -1;
  }

  Setup setupConfigurer;
  setupConfigurer.SetupQueryAcceleration(memoryPointer, dbData, recordSize,
                                         doc.GetRowCount());

  delete[] memoryPointer;
  return 0;
}
