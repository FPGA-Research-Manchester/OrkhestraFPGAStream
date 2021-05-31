#include "graph.hpp"

using easydspi::dbmstodspi::Graph;

std::string Graph::exportData() const {
  return stored_data_.at(0)->operation_type;
}

void Graph::insertData(std::string given_data) {
  stored_data_.at(0)->operation_type = given_data;
}