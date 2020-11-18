#pragma once
#include <vector>
#include <string>
#include <utility>
#include <cstdint>

struct TableData {
  std::vector<std::pair<std::string, int>> table_column_label_vector;
  std::vector<uint32_t> table_data_vector;

  auto operator==(const TableData& comparable_table) const -> bool {
    const bool same_columns =
        table_column_label_vector == comparable_table.table_column_label_vector;
    const bool same_data =
        table_data_vector == comparable_table.table_data_vector;
    return same_columns && same_data;
  }
};