#pragma once
#include <vector>
#include <string>
#include <utility>
#include <cstdint>

struct TableData {
  std::vector<std::pair<std::string, int>> table_column_label_vector;
  std::vector<uint32_t> table_data_vector;

  bool operator==(const TableData& rhs) const {
    const bool same_columns =
        table_column_label_vector == rhs.table_column_label_vector;
    const bool same_data = table_data_vector == rhs.table_data_vector;
    return same_columns && same_data;
  }
};