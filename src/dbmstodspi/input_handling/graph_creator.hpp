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

#include "graph_creator_interface.hpp"
#include "json_reader_interface.hpp"
#include "json_validator_interface.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::JSONReaderInterface;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Factory for creating query plan graphs.
 */
class GraphCreator : public GraphCreatorInterface {
 private:
  std::unique_ptr<JSONReaderInterface> json_reader_;
  std::unique_ptr<JSONValidatorInterface> json_validator_;

  static void PopulateGraphNodesMapWithJSONData(
      std::map<std::string, JSONReaderInterface::InputNodeParameters> &data,
      std::map<std::string, std::shared_ptr<QueryNode>> &graph_nodes_map,
      std::map<std::string, std::vector<std::string>> &previous_nodes,
      std::map<std::string, std::vector<std::string>> &next_nodes);
  static void LinkDependentNodes(
      std::map<std::string, std::shared_ptr<QueryNode>> &graph_nodes_map,
      std::map<std::string, std::vector<std::string>> &previous_nodes,
      std::map<std::string, std::vector<std::string>> &next_nodes);

 public:
  /**
   * @brief Constructor for making the graph creator with the given JSON reader
   * object.
   * @param json_reader Object to read JSON files with.
   * @param json_validator Object to check JSON formating.
   */
  explicit GraphCreator(std::unique_ptr<JSONReaderInterface> json_reader,
                        std::unique_ptr<JSONValidatorInterface> json_validator)
      : json_reader_{std::move(json_reader)},
        json_validator_{std::move(json_validator)} {};
  /**
   * @brief Read the given input_def file and make the query plan graph object.
   * @param graph_def_filename File containing the query plan information.
   * @return Query plan graph created form the given JSON.
   */
  auto MakeGraph(std::string graph_def_filename)
      -> std::unique_ptr<ExecutionPlanGraphInterface> override;
};
}  // namespace orkhestrafs::dbmstodspi