#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "query_scheduling_data.hpp"

using dbmstodspi::query_managing::query_scheduling_data::QueryNode;

namespace dbmstodspi::input_managing {
class GraphCreator {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;

 public:
  GraphCreator(std::unique_ptr<JSONReaderInterface> json_reader)
      : json_reader_{std::move(json_reader)} {};
  std::vector<QueryNode> makeGraph(
      std::string graph_def_filename);
};
}  // namespace easydspi::dbmstodspi