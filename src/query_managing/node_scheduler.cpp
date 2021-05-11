#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"

using namespace dbmstodspi::query_managing;

void NodeScheduler::FindAcceleratedQueryNodeSets(
    std::queue<std::pair<query_scheduling_data::ConfigurableModulesVector,
                         std::vector<query_scheduling_data::QueryNode>>>*
        accelerated_query_node_runs,
    std::vector<query_scheduling_data::QueryNode> starting_nodes,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<fpga_managing::operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library) {
  std::vector<query_scheduling_data::QueryNode> scheduled_queries;

  // Not checking for cycles nor for unsupported opperations
  while (!starting_nodes.empty()) {
    auto available_nodes_iterator =
        FindNextAvailableNode(scheduled_queries, starting_nodes);
    if (available_nodes_iterator == starting_nodes.end()) {
      throw std::runtime_error("Failed to schedule!");
    }

    query_scheduling_data::ConfigurableModulesVector current_set;
    std::vector<query_scheduling_data::QueryNode> current_query_nodes;

    CheckNodeForModuleSet(available_nodes_iterator, current_set,
                          current_query_nodes, scheduled_queries,
                          starting_nodes, supported_accelerator_bitstreams,
                          existing_modules_library);

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

// Method to remove next or previous nodes from a node once it has been
// scheduled
void NodeScheduler::RemoveLinkedNodes(
    std::vector<query_scheduling_data::QueryNode*>& linked_nodes,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes) {
  for (int i = 0; i < linked_nodes.size(); i++) {
    auto* linked_node = linked_nodes[i];
    if (linked_node != nullptr &&
        std::find(current_query_nodes.begin(), current_query_nodes.end(),
                  *linked_node) == current_query_nodes.end()) {
      linked_nodes[i] = nullptr;
    }
  }
}

// Function to find the minimum position for a node such that all the
// prerequisites are met
auto NodeScheduler::FindMinPosition(
    query_scheduling_data::QueryNode& current_node,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector)
    -> int {
  int min_position_index = 0;
  for (const auto& previous_node : current_node.previous_nodes) {
    if (previous_node != nullptr) {
      auto current_nodes_iterator =
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    *previous_node);
      if (current_nodes_iterator != current_query_nodes.end() &&
          previous_node->operation_type !=
              fpga_managing::operation_types::QueryOperationType::
                  kPassThrough) {
        auto current_modules_iterator = std::find_if(
            current_modules_vector.begin(), current_modules_vector.end(),
            [&](const fpga_managing::operation_types::QueryOperation&
                    operation) {
              return operation.operation_type == previous_node->operation_type;
            });
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

// Check recursively if the given node can be added to the set of nodes to be
// scheduled
void NodeScheduler::CheckNodeForModuleSet(
    std::vector<query_scheduling_data::QueryNode>::iterator&
        current_node_iterator,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    std::vector<query_scheduling_data::QueryNode>& scheduled_queries,
    std::vector<query_scheduling_data::QueryNode>& starting_nodes,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<fpga_managing::operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library) {
  auto current_node = *current_node_iterator;

  auto suitable_combination = FindSuitableModuleCombination(
      current_node, current_query_nodes, current_modules_vector,
      supported_accelerator_bitstreams, existing_modules_library);

  if (!suitable_combination.empty()) {
    current_query_nodes.push_back(current_node);
    current_modules_vector = suitable_combination;
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
                            starting_nodes, supported_accelerator_bitstreams,
                            existing_modules_library);
    }
  } else {
    std::advance(current_node_iterator, 1);
    if (current_node_iterator != starting_nodes.end()) {
      CheckNodeForModuleSet(current_node_iterator, current_modules_vector,
                            current_query_nodes, scheduled_queries,
                            starting_nodes, supported_accelerator_bitstreams,
                            existing_modules_library);
    }
  }
}

// Check if the desired module can be added to the existing combination
auto NodeScheduler::FindSuitableModuleCombination(
    query_scheduling_data::QueryNode& current_node,
    std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<fpga_managing::operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library)
    -> query_scheduling_data::ConfigurableModulesVector {
  int current_position = FindMinPosition(current_node, current_query_nodes,
                                         current_modules_vector);

  auto find_iterator =
      existing_modules_library.find(current_node.operation_type);
  if (find_iterator == existing_modules_library.end()) {
    throw std::runtime_error("Operation parameters not found!");
  }
  std::vector<std::vector<int>> current_module_possible_parameters =
      find_iterator->second;

  for (int module_position = current_position;
       module_position < current_modules_vector.size() + 1; module_position++) {
    std::vector<int> module_parameters;
    for (const auto& parameter_options : current_module_possible_parameters) {
      if (!parameter_options.empty()) {
        module_parameters.push_back(parameter_options.at(0));
      }
    }
    if (!module_parameters.empty()) {
      auto new_modules_vector = CheckModuleParameterSupport(
          module_parameters, current_modules_vector, module_position, 0,
          current_node.operation_type, current_module_possible_parameters,
          supported_accelerator_bitstreams);
      if (!new_modules_vector.empty()) {
        return new_modules_vector;
      }
    } else {
      auto new_modules_vector =
          CreateNewModulesVector(current_node.operation_type, module_position,
                                 current_modules_vector, module_parameters);
      if (IsModuleSetSupported(new_modules_vector,
                               supported_accelerator_bitstreams)) {
        return new_modules_vector;
      }
    }
  }
  return {};
}

// Recursive method to go through all of the module parameters to find the first
// supported one
auto dbmstodspi::query_managing::NodeScheduler::CheckModuleParameterSupport(
    std::vector<int> module_parameters,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector,
    int module_position, int parameter_option_index,
    fpga_managing::operation_types::QueryOperationType query_operation,
    std::vector<std::vector<int>> current_module_possible_parameters,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams)
    -> query_scheduling_data::ConfigurableModulesVector {
  for (const auto& parameter_value :
       current_module_possible_parameters.at(parameter_option_index)) {
    module_parameters[parameter_option_index] = parameter_value;
    if (parameter_option_index ==
        current_module_possible_parameters.size() - 1) {
      auto new_modules_vector =
          CreateNewModulesVector(query_operation, module_position,
                                 current_modules_vector, module_parameters);
      if (IsModuleSetSupported(new_modules_vector,
                               supported_accelerator_bitstreams)) {
        return new_modules_vector;
      }
    } else {
      auto new_modules_vector = CheckModuleParameterSupport(
          module_parameters, current_modules_vector, module_position,
          parameter_option_index + 1, query_operation,
          current_module_possible_parameters, supported_accelerator_bitstreams);
      if (!new_modules_vector.empty()) {
        return new_modules_vector;
      }
    }
  }

  return {};
}

// Add the desired module to the specified location.
auto NodeScheduler::CreateNewModulesVector(
    fpga_managing::operation_types::QueryOperationType query_operation,
    int current_position,
    query_scheduling_data::ConfigurableModulesVector current_modules_vector,
    std::vector<int> module_parameters)
    -> query_scheduling_data::ConfigurableModulesVector {
  if (query_operation ==
      fpga_managing::operation_types::QueryOperationType::kPassThrough) {
    return current_modules_vector;
  }
  if (current_modules_vector.empty()) {
    return {{query_operation, module_parameters}};
  }
  if (current_position == current_modules_vector.size()) {
    query_scheduling_data::ConfigurableModulesVector
        temp_current_modules_vector = current_modules_vector;
    temp_current_modules_vector.emplace_back(query_operation,
                                             module_parameters);
    return temp_current_modules_vector;
  }
  query_scheduling_data::ConfigurableModulesVector temp_current_modules_vector(
      current_modules_vector.begin(),
      current_modules_vector.begin() + current_position);
  temp_current_modules_vector.emplace_back(query_operation, module_parameters);
  temp_current_modules_vector.insert(
      temp_current_modules_vector.begin() + current_position + 1,
      current_modules_vector.begin() + current_position,
      current_modules_vector.end());
  return temp_current_modules_vector;
}

// Check if the module set does have a corresponding bitstream in the bitstreams
// library
auto NodeScheduler::IsModuleSetSupported(
    const query_scheduling_data::ConfigurableModulesVector& module_set,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams) -> bool {
  return supported_accelerator_bitstreams.find(module_set) !=
         supported_accelerator_bitstreams.end();
}

// Check if the given node is in the given vector.
auto NodeScheduler::IsNodeIncluded(
    const std::vector<query_scheduling_data::QueryNode>& node_vector,
    const query_scheduling_data::QueryNode& searched_node) -> bool {
  return std::any_of(
      node_vector.begin(), node_vector.end(),
      [&](const query_scheduling_data::QueryNode& scheduled_node) {
        return scheduled_node == searched_node;
      });
}

// Check if all required nodes have been scheduled already
auto NodeScheduler::IsNodeAvailable(
    const std::vector<query_scheduling_data::QueryNode>& scheduled_nodes,
    const query_scheduling_data::QueryNode& current_node) -> bool {
  for (const auto& required_node : current_node.previous_nodes) {
    if (required_node && !IsNodeIncluded(scheduled_nodes, *required_node)) {
      return false;
    }
  }
  return true;
}

// Find function for getting an iterator for a node which is available.
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
