#include "node_scheduler.hpp"

#include <stdexcept>

// Very incorrect logic here for now!
void NodeScheduler::FindAcceleratedQueryNodeSets(
    std::queue<std::pair<query_scheduling_data::ConfigurableModuleSet,
                         std::vector<query_scheduling_data::QueryNode>>>*
        accelerated_query_node_sets,
    std::vector<query_scheduling_data::QueryNode> starting_nodes) {
  for (const auto& query_node : starting_nodes) {
    if (!IsOperatorSupported(query_node.operation_type)) {
      throw std::runtime_error("Unsopported operation!");
    }

    query_scheduling_data::ConfigurableModuleSet current_set;
    std::vector<query_scheduling_data::QueryNode> current_query_nodes;
    for (const auto& size : query_scheduling_data::available_modules
                                .find(query_node.operation_type)
                                ->second) {
      current_set.insert({query_node.operation_type, size});
      if (!IsModuleSetSupported({current_set})) {
        throw std::runtime_error("Wrong module set found!");
      } else {
        current_query_nodes.push_back(query_node);
      }
    }
    accelerated_query_node_sets->push({current_set, current_query_nodes});
  }
}

auto NodeScheduler::IsOperatorSupported(
    operation_types::QueryOperation query_operation) -> bool {
  return query_scheduling_data::available_modules.find(query_operation) !=
         query_scheduling_data::available_modules.end();
}

auto NodeScheduler::IsModuleSetSupported(
    query_scheduling_data::ConfigurableModuleSet module_set) -> bool {
  return query_scheduling_data::corresponding_accelerator_bitstreams.find(
             module_set) !=
         query_scheduling_data::corresponding_accelerator_bitstreams.end();
}
