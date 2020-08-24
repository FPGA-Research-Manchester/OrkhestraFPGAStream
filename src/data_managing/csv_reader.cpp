#include "csv_reader.hpp"

void CSVReader::AddCSVRows(std::string file_name,
                           std::vector<std::vector<std::string>>& results_vector) {
  rapidcsv::Document doc(file_name, rapidcsv::LabelParams(-1, -1));
  for (int row_number = 0; row_number < doc.GetRowCount(); row_number++) {
    results_vector.push_back(doc.GetRow<std::string>(row_number));
  }
}
