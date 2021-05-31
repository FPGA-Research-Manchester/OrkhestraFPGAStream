#include "graph_creator.hpp"

#include "graph.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::ExecutionPlanNode;
using easydspi::core_interfaces::NodeOperationParameters;
using easydspi::dbmstodspi::Graph;
using easydspi::dbmstodspi::GraphCreator;

std::unique_ptr<ExecutionPlanGraphInterface> GraphCreator::makeGraph(
    std::string input_def_filename) {
  auto data = json_reader_->readInputDefinition(input_def_filename);

  using ParamsMap = std::map<std::string, std::vector<std::vector<int>>>;

  std::string input_field = "input";
  std::string output_field = "output";
  std::string operation_field = "operation";
  std::string previous_node_field = "previous_node";
  std::string next_node_field = "next_node";
  std::string all_operation_parameters_field = "operation_parameters";
  std::string input_stream_params_field = "input_stream_params";
  std::string output_stream_params = "output_stream_params";
  std::string operation_params_field = "operation_params";

  std::map<std::string, std::shared_ptr<ExecutionPlanNode>> graph_nodes_map;
  std::map<std::string, std::string> previous_nodes;
  std::map<std::string, std::string> next_nodes;

  for (auto const& [node_name, node_parameters] : data) {
    ParamsMap all_operation_parameters_map =
        std::get<ParamsMap>(node_parameters.at(all_operation_parameters_field));
    NodeOperationParameters all_operation_parameters{
        all_operation_parameters_map.at(input_stream_params_field),
        all_operation_parameters_map.at(output_stream_params),
        all_operation_parameters_map.at(operation_params_field)};
    graph_nodes_map.insert(
        {node_name,
         std::make_shared<ExecutionPlanNode>(
             std::get<std::string>(node_parameters.at(input_field)),
             std::get<std::string>(node_parameters.at(output_field)), nullptr,
             nullptr,
             std::get<std::string>(node_parameters.at(operation_field)),
             all_operation_parameters)});

    auto search_previous = node_parameters.find(previous_node_field);
    if (search_previous != node_parameters.end()) {
      previous_nodes.insert(
          {node_name, std::get<std::string>(search_previous->second)});
    }
    auto search_next = node_parameters.find(next_node_field);
    if (search_next != node_parameters.end()) {
      next_nodes.insert(
          {node_name, std::get<std::string>(search_next->second)});
    }
  }

  for (auto const& [node_name, node] : graph_nodes_map) {
    auto search_previous = previous_nodes.find(node_name);
    if (search_previous != previous_nodes.end()) {
      node->previous_node=graph_nodes_map.at(search_previous->second);
    }
    auto search_next = next_nodes.find(node_name);
    if (search_next != next_nodes.end()) {
      node->next_node = graph_nodes_map.at(search_next->second);
    }
  }

  std::vector<std::shared_ptr<ExecutionPlanNode>> graph_nodes;
  for (auto const& [node_name, node] : graph_nodes_map) {
    graph_nodes.push_back(std::move(node));
  }

  return std::make_unique<Graph>(graph_nodes);
}