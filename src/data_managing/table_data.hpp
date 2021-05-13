#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace dbmstodspi::data_managing {

/**
 * @brief Struct to hold the information about the table data.
 */
struct TableData {
  /// Vector to hold data about column data types and sizes.
  std::vector<std::pair<std::string, int>> table_column_label_vector;
  /// Vector to hold the table data in integer format.
  std::vector<uint32_t> table_data_vector;

  auto operator==(const TableData& comparable_table) const -> bool {
    const bool same_columns =
        table_column_label_vector == comparable_table.table_column_label_vector;
    const bool same_data =
        table_data_vector == comparable_table.table_data_vector;
    return same_columns && same_data;
  }
};

}  // namespace dbmstodspi::data_managing