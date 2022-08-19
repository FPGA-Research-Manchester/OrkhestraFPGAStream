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

#include "graph_creator.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "graph.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::kSupportedFunctions;
using orkhestrafs::dbmstodspi::Graph;
using orkhestrafs::dbmstodspi::GraphCreator;

auto GraphCreator::MakeGraph(std::string graph_def_filename)
    -> std::unique_ptr<ExecutionPlanGraphInterface> {

  if (graph_def_filename.empty()) {
    std::vector<std::unique_ptr<QueryNode>> empty_graph;
    return std::make_unique<Graph>(std::move(empty_graph));
  }

  std::map<std::string, QueryNode*> graph_nodes_map;
  std::map<std::string, std::vector<std::string>> previous_nodes;
  std::map<std::string, std::vector<std::string>> next_nodes;

  // Make a vector in populate graph and move them into the graph.
  // previous nodes and next nodes is already good.
  // Basically instead of giving graph nodes_map to populate you get the
  // unique_ptr back.

  // Then you can return all of the pointers and make a map
  // Then return the Graph

  auto data = json_reader_->ReadInputDefinition(std::move(graph_def_filename));
  auto graph =
      PopulateGraphNodesMapWithJSONData(data, previous_nodes, next_nodes);

  for (const auto& node_ptr : graph->GetAllNodesPtrs()) {
    graph_nodes_map.insert({node_ptr->node_name, node_ptr});
  }

  LinkDependentNodes(graph_nodes_map, previous_nodes, next_nodes);

  return std::move(graph);
}

void GraphCreator::LinkDependentNodes(
    std::map<std::string, QueryNode*>& graph_nodes_map,
    std::map<std::string, std::vector<std::string>>& previous_nodes,
    std::map<std::string, std::vector<std::string>>& next_nodes) {
  // TODO(Kaspar): Improve performance of quick check
  for (const auto& [node_name, dependent_nodes] : previous_nodes) {
    if (dependent_nodes.empty()) {
      throw std::runtime_error("Previous nodes left empty (no null either)!");
    }
    for (const auto& node : dependent_nodes) {
      if (!node.empty() &&
          graph_nodes_map.find(node) == graph_nodes_map.end()) {
        throw std::runtime_error("Linked previous node doesn't exist!");
      }
    }
  }
  for (const auto& [node_name, dependent_nodes] : next_nodes) {
    if (dependent_nodes.empty()) {
      throw std::runtime_error("Previous nodes left empty (no null either)!");
    }
    for (const auto& node : dependent_nodes) {
      if (!node.empty() &&
          graph_nodes_map.find(node) == graph_nodes_map.end()) {
        throw std::runtime_error("Linked next node doesn't exist!");
      }
    }
  }

  for (auto& [node_name, node] : graph_nodes_map) {
    auto search_previous = previous_nodes.find(node_name);
    if (search_previous != previous_nodes.end()) {
      if (search_previous->second.size() !=
          node->given_input_data_definition_files.size()) {
        throw std::runtime_error(
            "Incorrect number of input file definitions found!");
      }
      for (int i = 0; i < search_previous->second.size(); i++) {
        if (!search_previous->second[i].empty()) {
          if (!node->given_input_data_definition_files[i].empty()) {
            throw std::runtime_error("Input file not required!");
          }
          node->previous_nodes.push_back(
              graph_nodes_map.at(search_previous->second[i]));
        } else {
          if (node->given_input_data_definition_files[i].empty()) {
            throw std::runtime_error("Input file required!");
          }
          node->previous_nodes.push_back(nullptr);
        }
      }
    }
    auto search_next = next_nodes.find(node_name);
    if (search_next != next_nodes.end()) {
      for (auto const& next_node_name : search_next->second) {
        if (!next_node_name.empty()) {
          node->next_nodes.push_back(graph_nodes_map.at(next_node_name));
        } else {
          node->next_nodes.push_back(nullptr);
        }
      }
    }
  }
}

auto GraphCreator::PopulateGraphNodesMapWithJSONData(
    std::map<std::string, JSONReaderInterface::InputNodeParameters>& data,
    std::map<std::string, std::vector<std::string>>& previous_nodes,
    std::map<std::string, std::vector<std::string>>& next_nodes)
    -> std::unique_ptr<ExecutionPlanGraphInterface> {
  using ParamsMap = std::map<std::string, std::vector<std::vector<int>>>;
  using orkhestrafs::core_interfaces::query_scheduling_data::
      NodeOperationParameters;

  std::string input_field = "input";
  std::string output_field = "output";
  std::string operation_field = "operation";
  std::string previous_nodes_field = "previous_nodes";
  std::string next_nodes_field = "next_nodes";
  std::string all_operation_parameters_field = "operation_parameters";
  std::string input_stream_params_field = "input_stream_params";
  std::string output_stream_params = "output_stream_params";
  std::string operation_params_field = "operation_params";

  std::vector<std::unique_ptr<QueryNode>> all_nodes;

  for (auto const& [node_name, node_parameters] : data) {
    ParamsMap all_operation_parameters_map =
        std::get<ParamsMap>(node_parameters.at(all_operation_parameters_field));
    NodeOperationParameters all_operation_parameters{
        all_operation_parameters_map.at(input_stream_params_field),
        all_operation_parameters_map.at(output_stream_params),
        all_operation_parameters_map.at(operation_params_field)};

    auto output_filenames =
        std::get<std::vector<std::string>>(node_parameters.at(output_field));
    std::vector<bool> is_checked;
    is_checked.reserve(output_filenames.size());
    for (const auto& filename : output_filenames) {
      is_checked.push_back(!filename.empty());
    }

    all_nodes.push_back(std::move(std::make_unique<QueryNode>(
        std::get<std::vector<std::string>>(node_parameters.at(input_field)),
        output_filenames,
        kSupportedFunctions.at(
            std::get<std::string>(node_parameters.at(operation_field))),
        std::vector<QueryNode*>(), std::vector<QueryNode*>(),
        all_operation_parameters, node_name, is_checked)));

    auto search_previous = node_parameters.find(previous_nodes_field);
    if (search_previous != node_parameters.end()) {
      previous_nodes.insert({node_name, std::get<std::vector<std::string>>(
                                            search_previous->second)});
    }
    auto search_next = node_parameters.find(next_nodes_field);
    if (search_next != node_parameters.end()) {
      next_nodes.insert(
          {node_name, std::get<std::vector<std::string>>(search_next->second)});
    }
  }
  return std::make_unique<Graph>(std::move(all_nodes));
}
