#include "csv_reader.hpp"

using namespace dbmstodspi::data_managing;

auto CSVReader::ReadTableData(const std::string& filename)
    -> std::vector<std::vector<std::string>> {
  rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1));
  std::vector<std::vector<std::string>> data_rows;
  data_rows.reserve(doc.GetRowCount());
  for (int row_number = 0; row_number < doc.GetRowCount(); row_number++) {
    data_rows.push_back(doc.GetRow<std::string>(row_number));
  }
  return data_rows;
}
