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

#pragma once

#include <memory>

#include "json_reader_interface.hpp"
#include "query_scheduling_data.hpp"

using dbmstodspi::query_managing::query_scheduling_data::QueryNode;

namespace dbmstodspi::input_managing {
/**
 * @brief Factory for creating query plan graphs.
*/
class GraphCreator {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;
  static void PopulateGraphNodesMapWithJSONData(
      std::map<std::string, dbmstodspi::input_managing::JSONReaderInterface::
                                InputNodeParameters> &data,
      std::map<
          std::string,
          std::shared_ptr<
              dbmstodspi::query_managing::query_scheduling_data::QueryNode>>
          &graph_nodes_map,
      std::map<std::string, std::vector<std::string>> &previous_nodes,
      std::map<std::string, std::vector<std::string>> &next_nodes);
  static void LinkDependentNodes(
      std::map<
          std::string,
          std::shared_ptr<
              dbmstodspi::query_managing::query_scheduling_data::QueryNode>>
          &graph_nodes_map,
      std::map<std::string, std::vector<std::string>> &previous_nodes,
      std::map<std::string, std::vector<std::string>> &next_nodes);

 public:
  /**
   * @brief Constructor for making the graph creator with the given JSON reader object.
   * @param json_reader Object to read JSON files with.
  */
  explicit GraphCreator(std::unique_ptr<JSONReaderInterface> json_reader)
      : json_reader_{std::move(json_reader)} {};
  /**
   * @brief Read the given input_def file and make the query plan graph object.
   * @param input_def_filename File containing the query plan information.
   * @return Query plan graph created form the given JSON.
  */
  auto MakeGraph(std::string input_def_filename)
      -> std::vector<std::shared_ptr<QueryNode>>;
};
}  // namespace dbmstodspi::input_managing