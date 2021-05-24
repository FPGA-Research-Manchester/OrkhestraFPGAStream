#include "graph.hpp"

using easydspi::dbmstodspi::Graph;

std::string Graph::exportData() { return stored_data_; }

void Graph::insertData(std::string given_data) { stored_data_ = given_data; }