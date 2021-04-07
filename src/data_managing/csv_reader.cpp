#include "csv_reader.hpp"

using namespace dbmstodspi::data_managing;

void CSVReader::ReadTableData(
    const std::string& filename, std::vector<std::string>& header_row,
    std::vector<std::vector<std::string>>& read_rows) {
  rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1));
  header_row = doc.GetRow<std::string>(0);
  read_rows.reserve(doc.GetRowCount() - 1);
  for (int row_number = 1; row_number < doc.GetRowCount(); row_number++) {
    read_rows.push_back(doc.GetRow<std::string>(row_number));
  }
}
