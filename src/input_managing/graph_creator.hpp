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
  explicit GraphCreator(std::unique_ptr<JSONReaderInterface> json_reader)
      : json_reader_{std::move(json_reader)} {};
  auto MakeGraph(std::string input_def_filename)
      -> std::vector<std::shared_ptr<QueryNode>>;
};
}  // namespace dbmstodspi::input_managing