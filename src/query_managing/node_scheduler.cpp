#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

void NodeScheduler::FindAcceleratedQueryNodeSets(
    std::queue<std::pair<query_scheduling_data::ConfigurableModuleSet,
                         std::vector<query_scheduling_data::QueryNode>>>*
        accelerated_query_node_sets,
    std::vector<query_scheduling_data::QueryNode> starting_nodes) {
  std::vector<query_scheduling_data::QueryNode> scheduled_queries;

  // Not checking for cycles nor for unsupported opperations
  while (starting_nodes.size() != 0) {
    // Should find the first element really
    auto available_nodes_iterator =
        FindNextAvailableNode(scheduled_queries, starting_nodes);
    if (available_nodes_iterator == starting_nodes.end()) {
      throw std::runtime_error("Failed to schedule!");
    }

    query_scheduling_data::ConfigurableModuleSet current_set;
    std::vector<query_scheduling_data::QueryNode> current_query_nodes;

    CheckNodeForModuleSet(available_nodes_iterator, current_set,
                          current_query_nodes, scheduled_queries,
                          starting_nodes);

    if (current_query_nodes.empty()) {
      throw std::runtime_error("Failed to schedule!");
    }
    accelerated_query_node_sets->push({current_set, current_query_nodes});
  }
}

// The algorithm still doesn't check for multiple sizes and doesn't prioritise
// pipelining operations
void NodeScheduler::CheckNodeForModuleSet(
    std::vector<query_scheduling_data::QueryNode>::iterator& iterator,
    query_scheduling_data::ConfigurableModuleSet& current_set,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    std::vector<query_scheduling_data::QueryNode>& scheduled_queries,
    std::vector<query_scheduling_data::QueryNode>& starting_nodes) {
  auto current_node = *iterator;
  int chosen_module_size =
      FindModuleSize(current_node.operation_type, current_set);
  if (chosen_module_size != -1) {
    current_query_nodes.push_back(current_node);
    current_set.insert({current_node.operation_type, chosen_module_size});
    scheduled_queries.push_back(current_node);

    if (!current_node.next_nodes.empty()) {
      for (const auto& next_node : current_node.next_nodes) {
        if (!IsNodeIncluded(starting_nodes, *next_node) &&
            IsNodeAvailable(scheduled_queries, *next_node)) {
          starting_nodes.push_back(*next_node);
        }
      }
    }

    starting_nodes.erase(iterator);
    auto new_iterator =
        FindNextAvailableNode(scheduled_queries, starting_nodes);
    if (new_iterator != starting_nodes.end()) {
      CheckNodeForModuleSet(new_iterator, current_set, current_query_nodes,
                            scheduled_queries, starting_nodes);
    }
  } else {
    std::advance(iterator, 1);
    if (iterator != starting_nodes.end()) {
      CheckNodeForModuleSet(iterator, current_set, current_query_nodes,
                            scheduled_queries, starting_nodes);
    }
  }
}

auto NodeScheduler::IsModuleSetSupported(
    query_scheduling_data::ConfigurableModuleSet module_set) -> bool {
  return query_scheduling_data::supported_accelerator_bitstreams.find(
             module_set) !=
         query_scheduling_data::supported_accelerator_bitstreams.end();
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

auto NodeScheduler::IsNodeAvailable(
    std::vector<query_scheduling_data::QueryNode> scheduled_nodes,
    query_scheduling_data::QueryNode current_node) -> bool {
  for (const auto& required_node : current_node.previous_nodes) {
    if (!IsNodeIncluded(scheduled_nodes, *required_node)) {
      return false;
    }
  }
  return true;
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

auto NodeScheduler::FindNextAvailableNode(
    std::vector<query_scheduling_data::QueryNode>& already_scheduled_nodes,
    std::vector<query_scheduling_data::QueryNode>& starting_nodes)
    -> std::vector<query_scheduling_data::QueryNode>::iterator {
  return std::find_if(starting_nodes.begin(), starting_nodes.end(),
                      [&](const query_scheduling_data::QueryNode& leaf_node) {
                        return IsNodeAvailable(already_scheduled_nodes,
                                               leaf_node);
                      });
}

auto NodeScheduler::IsOperatorSupported(
    operation_types::QueryOperation query_operation) -> bool {
  return query_scheduling_data::available_modules.find(query_operation) !=
         query_scheduling_data::available_modules.end();
}
