#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

void NodeScheduler::FindAcceleratedQueryNodeSets(
    std::queue<std::pair<query_scheduling_data::ConfigurableModuleSet,
                         std::vector<query_scheduling_data::QueryNode>>>*
        accelerated_query_node_sets,
    std::vector<query_scheduling_data::QueryNode> starting_nodes) {
  std::vector<query_scheduling_data::QueryNode> scheduled_queries;

  while (starting_nodes.size() != 0) {
    auto current_node = starting_nodes[0];
    // Commented out for now to use duplicate query nodes.
    /*if (IsNodeIncluded(scheduled_queries, current_node)) {
      throw std::runtime_error("Cycle in the query tree found!");
    }*/
    if (!IsOperatorSupported(current_node.operation_type)) {
      throw std::runtime_error("Unsopported operation!");
    }
    starting_nodes.erase(starting_nodes.begin());

    query_scheduling_data::ConfigurableModuleSet current_set;
    std::vector<query_scheduling_data::QueryNode> current_query_nodes;

    int chosen_module_size =
        FindModuleSize(current_node.operation_type, current_set);
    if (chosen_module_size != -1) {
      current_query_nodes.push_back(current_node);
      current_set.insert({current_node.operation_type, chosen_module_size});
      scheduled_queries.push_back(current_node);
    }

    if (current_query_nodes.empty()) {
      throw std::runtime_error("Failed to schedule!");
    }
    accelerated_query_node_sets->push({current_set, current_query_nodes});
  }

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

auto NodeScheduler::IsNodeIncluded(
    std::vector<query_scheduling_data::QueryNode> node_vector,
    query_scheduling_data::QueryNode searched_node) -> bool {
  return std::any_of(
      node_vector.begin(), node_vector.end(),
      [&](const query_scheduling_data::QueryNode scheduled_node) {
        return scheduled_node == searched_node;
      });
}

auto NodeScheduler::FindModuleSize(
    operation_types::QueryOperation query_operation,
    query_scheduling_data::ConfigurableModuleSet current_set) -> int {
  int chosen_size = -1;

  for (const auto& current_size :
       query_scheduling_data::available_modules.find(query_operation)->second) {
    auto possible_set = current_set;
    possible_set.insert({query_operation, current_size});
    if (IsModuleSetSupported(possible_set) && current_size > chosen_size) {
      chosen_size = current_size;
    }
  }

  return chosen_size;
}
