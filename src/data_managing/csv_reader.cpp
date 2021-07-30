/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "csv_reader.hpp"

using namespace dbmstodspi::data_managing;

auto CSVReader::ReadTableData(const std::string& filename, char separator)
-> std::vector<std::vector<std::string>> {
	rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1),
		rapidcsv::SeparatorParams(separator));
	std::vector<std::vector<std::string>> data_rows;
	data_rows.reserve(doc.GetRowCount());
	for (int row_number = 0; row_number < doc.GetRowCount(); row_number++) {
		data_rows.push_back(doc.GetRow<std::string>(row_number));
	}
	return data_rows;
}
