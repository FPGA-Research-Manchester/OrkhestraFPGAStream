#include "csv_reader.hpp"

#include <fstream>
#include <iostream>

using namespace dbmstodspi::data_managing;

auto CSVReader::ReadBigFile(const std::string& filename, char separator,
                            int& rows_already_read)
    -> std::vector<std::vector<std::string>> {
  std::ifstream big_file(filename);  // Make this in beginning of read.
  std::vector<std::vector<std::string>> data;
  std::string line;
  int max_rows = 2000000;
  int row_counter = 0;
  while (std::getline(big_file, line)) {
    row_counter++;
    if (row_counter > rows_already_read) {
      std::vector<std::string> tokens;
      std::stringstream linestream(line);
      std::string intermediate;
      while (getline(linestream, intermediate, separator)) {
        if (!intermediate.empty() &&
            intermediate[intermediate.size() - 1] == '\r')
          intermediate.erase(intermediate.size() - 1);
        tokens.push_back(intermediate);
      }
      data.push_back(tokens);
    }
    if (row_counter >= rows_already_read + max_rows) {
      rows_already_read = row_counter;
      return data;
    }
  }
  rows_already_read = row_counter;
  return data;
}

auto CSVReader::ReadTableData(const std::string& filename, char separator,
                              int& rows_already_read)
    -> std::vector<std::vector<std::string>> {
  /*rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1),
                         rapidcsv::SeparatorParams(separator));
  std::vector<std::vector<std::string>> data_rows;*/

  ////data_rows.reserve(doc.GetRowCount());
  // data_rows.reserve(max_rows);
  // for (int row_number = rows_already_read;
  //     row_number < doc.GetRowCount() &&
  //     row_number < rows_already_read + max_rows;
  //     row_number++) {
  //  data_rows.push_back(doc.GetRow<std::string>(row_number));
  //  rows_already_read++;
  //}
  return ReadBigFile(filename, separator, rows_already_read);
}
