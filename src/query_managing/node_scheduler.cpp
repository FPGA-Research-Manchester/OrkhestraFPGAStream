#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"

using namespace dbmstodspi::query_managing;

void NodeScheduler::FindAcceleratedQueryNodeSets(
    std::queue<std::pair<query_scheduling_data::ConfigurableModulesVector,
                         std::vector<query_scheduling_data::QueryNode>>>*
        accelerated_query_node_runs,
    std::vector<query_scheduling_data::QueryNode> starting_nodes) {
  std::vector<query_scheduling_data::QueryNode> scheduled_queries;

  // Not checking for cycles nor for unsupported opperations
  while (starting_nodes.size() != 0) {
    // First time finds first element but next loop might be different when the
    // next node has multiple dependencies. But since starting nodes only
    // includes available nodes it should always pick the first one!
    auto available_nodes_iterator =
        FindNextAvailableNode(scheduled_queries, starting_nodes);
    if (available_nodes_iterator == starting_nodes.end()) {
      throw std::runtime_error("Failed to schedule!");
    }

    query_scheduling_data::ConfigurableModulesVector current_set;
    std::vector<query_scheduling_data::QueryNode> current_query_nodes;

    CheckNodeForModuleSet(available_nodes_iterator, current_set,
                          current_query_nodes, scheduled_queries,
                          starting_nodes);

    if (current_query_nodes.empty()) {
      throw std::runtime_error("Failed to schedule!");
    }

    for (auto& node : current_query_nodes) {
      RemoveLinkedNodes(node.next_nodes, current_query_nodes);
      RemoveLinkedNodes(node.previous_nodes, current_query_nodes);
    }

    accelerated_query_node_runs->push({current_set, current_query_nodes});
  }
}

void NodeScheduler::RemoveLinkedNodes(
    std::vector<query_scheduling_data::QueryNode*>& linked_nodes,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes) {
  for (int i = 0; i < linked_nodes.size(); i++) {
    auto linked_node = linked_nodes[i];
    if (linked_node &&
        std::find(current_query_nodes.begin(), current_query_nodes.end(),
                  *linked_node) == current_query_nodes.end()) {
      linked_nodes[i] = nullptr;
    }
  }
}

auto NodeScheduler::FindMinPosition(
    query_scheduling_data::QueryNode& current_node,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector)
    -> int {
  // Need to think of pass-through.
  int min_position_index = 0;
  for (const auto& previous_node : current_node.previous_nodes) {
    if (previous_node) {
      auto current_nodes_iterator =
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    *previous_node);
      if (current_nodes_iterator != current_query_nodes.end() &&
          previous_node->operation_type !=
              fpga_managing::operation_types::QueryOperation::kPassThrough) {
        auto current_modules_iterator = std::find(
            current_modules_vector.begin(), current_modules_vector.end(),
            previous_node->operation_type);
        if (current_modules_iterator == current_modules_vector.end()) {
          throw std::runtime_error("Something went wrong with scheduling!");
        }
        int current_position_index =
            current_modules_iterator - current_modules_vector.begin();
        if (current_position_index > min_position_index) {
          min_position_index = current_position_index;
        }
      }
    }
  }
  return min_position_index;
}

// The algorithm still doesn't check for multiple sizes and doesn't prioritise
// pipelining operations. The algorithm also doesn't take into account memory
// space or max stream count.
void NodeScheduler::CheckNodeForModuleSet(
    std::vector<query_scheduling_data::QueryNode>::iterator&
        current_node_iterator,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    std::vector<query_scheduling_data::QueryNode>& scheduled_queries,
    std::vector<query_scheduling_data::QueryNode>& starting_nodes) {
  auto current_node = *current_node_iterator;

  int suitable_position = FindSuitableModulePosition(
      current_node, current_query_nodes, current_modules_vector);

  if (suitable_position != -1) {
    current_query_nodes.push_back(current_node);
    current_modules_vector = CreateNewModulesVector(
        current_node.operation_type, suitable_position, current_modules_vector);
    scheduled_queries.push_back(current_node);

    starting_nodes.erase(current_node_iterator);

    if (!current_node.next_nodes.empty()) {
      for (const auto& next_node : current_node.next_nodes) {
        if (next_node && !IsNodeIncluded(starting_nodes, *next_node) &&
            IsNodeAvailable(scheduled_queries, *next_node)) {
          starting_nodes.push_back(*next_node);
        }
      }
    }

    auto new_iterator =
        FindNextAvailableNode(scheduled_queries, starting_nodes);
    if (new_iterator != starting_nodes.end()) {
      CheckNodeForModuleSet(new_iterator, current_modules_vector,
                            current_query_nodes, scheduled_queries,
                            starting_nodes);
    }
  } else {
    std::advance(current_node_iterator, 1);
    if (current_node_iterator != starting_nodes.end()) {
      CheckNodeForModuleSet(current_node_iterator, current_modules_vector,
                            current_query_nodes, scheduled_queries,
                            starting_nodes);
    }
  }
}

auto NodeScheduler::FindSuitableModulePosition(
    query_scheduling_data::QueryNode& current_node,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector)
    -> int {
  int current_position = FindMinPosition(current_node, current_query_nodes,
                                         current_modules_vector) -
                         1;

  bool is_suitable_set_found = false;
  while (!is_suitable_set_found &&
         current_position != current_modules_vector.size()) {
    current_position++;
    auto new_modules_vector = CreateNewModulesVector(
        current_node.operation_type, current_position, current_modules_vector);
    is_suitable_set_found = IsModuleSetSupported(new_modules_vector);
  }
  if (!is_suitable_set_found) {
    return -1;
  }
  return current_position;
}

auto NodeScheduler::CreateNewModulesVector(
    fpga_managing::operation_types::QueryOperation query_operation,
    int current_position,
    query_scheduling_data::ConfigurableModulesVector current_modules_vector)
    -> query_scheduling_data::ConfigurableModulesVector {
  if (query_operation ==
      fpga_managing::operation_types::QueryOperation::kPassThrough) {
    return current_modules_vector;
  }
  if (current_modules_vector.empty()) {
    return {query_operation};
  }
  if (current_position == current_modules_vector.size()) {
    query_scheduling_data::ConfigurableModulesVector
        temp_current_modules_vector = current_modules_vector;
    temp_current_modules_vector.push_back(query_operation);
    return temp_current_modules_vector;
  }
  query_scheduling_data::ConfigurableModulesVector temp_current_modules_vector(
      current_modules_vector.begin(),
      current_modules_vector.begin() + current_position);
  temp_current_modules_vector.push_back(query_operation);
  temp_current_modules_vector.insert(
      temp_current_modules_vector.begin() + current_position + 1,
      current_modules_vector.begin() + current_position,
      current_modules_vector.end());
  return temp_current_modules_vector;
}

auto NodeScheduler::IsModuleSetSupported(
    query_scheduling_data::ConfigurableModulesVector module_set) -> bool {
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
    if (required_node && !IsNodeIncluded(scheduled_nodes, *required_node)) {
      return false;
    }
  }
  return true;
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
