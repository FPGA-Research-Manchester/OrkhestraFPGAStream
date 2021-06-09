#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"

using namespace dbmstodspi::query_managing;
using namespace dbmstodspi::fpga_managing;

auto NodeScheduler::FindAcceleratedQueryNodeSets(
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        starting_nodes,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library)
    -> std::queue<std::pair<
        query_scheduling_data::ConfigurableModulesVector,
        std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>>> {
  std::queue<
      std::pair<query_scheduling_data::ConfigurableModulesVector,
                std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>>>
      query_node_runs_queue;

  std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
      scheduled_queries;

  // Not checking for cycles nor for unsupported opperations
  while (!starting_nodes.empty()) {
    int node_index = 0;

    query_scheduling_data::ConfigurableModulesVector current_set;
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        current_query_nodes;

    CheckNodeForModuleSet(node_index, current_set, current_query_nodes,
                          scheduled_queries, starting_nodes,
                          supported_accelerator_bitstreams,
                          existing_modules_library);

    if (current_query_nodes.empty()) {
      throw std::runtime_error("Failed to schedule!");
    }

    RemoveExternalLinks(current_query_nodes);

    query_node_runs_queue.push({current_set, current_query_nodes});
  }
  return query_node_runs_queue;
}

// Method to remove next or previous nodes from a node once it has been
// scheduled
void NodeScheduler::RemoveExternalLinks(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        current_query_nodes) {
  for (const auto& node : current_query_nodes) {
    for (auto& linked_node : node->next_nodes) {
      if (linked_node &&
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    linked_node) == current_query_nodes.end()) {
        linked_node = nullptr;
      }
    }
    for (auto& linked_node : node->previous_nodes) {
      auto observed_node = linked_node.lock();
      if (observed_node &&
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    observed_node) == current_query_nodes.end()) {
        linked_node = std::weak_ptr<query_scheduling_data::QueryNode>();
      }
    }
  }
}

// Function to find the minimum position for a node such that all the
// prerequisites are met
auto NodeScheduler::FindMinPosition(
    const query_scheduling_data::QueryNode* current_node,

    const std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    const query_scheduling_data::ConfigurableModulesVector&
        current_modules_vector) -> int {
  int min_position_index = 0;
  for (const auto& previous_node : current_node->previous_nodes) {
    auto observed_node = previous_node.lock();
    if (observed_node) {
      auto current_nodes_iterator =
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    *observed_node);
      if (current_nodes_iterator != current_query_nodes.end() &&
          observed_node->operation_type !=
              fpga_managing::operation_types::QueryOperationType::
                  kPassThrough) {
        auto current_modules_iterator = std::find_if(
            current_modules_vector.begin(), current_modules_vector.end(),
            [&](const fpga_managing::operation_types::QueryOperation&
                    operation) {
              return operation.operation_type == observed_node->operation_type;
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
    int node_index,
    query_scheduling_data::ConfigurableModulesVector& current_modules_vector,
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        current_query_nodes,
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        scheduled_queries,
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        starting_nodes,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<fpga_managing::operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library) {
  auto current_node = starting_nodes[node_index];

  auto suitable_combination = FindSuitableModuleCombination(
      current_node.get(), CreateReferenceVector(current_query_nodes),
      current_modules_vector, supported_accelerator_bitstreams,
      existing_modules_library);

  // The logic with pointers has to get cleaned up!
  if (!suitable_combination.empty()) {
    scheduled_queries.push_back(current_node);
    current_query_nodes.push_back(current_node);
    current_modules_vector = suitable_combination;

    if (!current_node->next_nodes.empty()) {
      for (const auto& next_node : current_node->next_nodes) {
        if (next_node &&
            !IsNodeIncluded(CreateReferenceVector(starting_nodes),
                            *next_node) &&
            IsNodeAvailable(CreateReferenceVector(scheduled_queries),
                            *next_node)) {
          starting_nodes.push_back(next_node);
        }
      }
    }

    starting_nodes.erase(starting_nodes.begin() + node_index);

    if (!starting_nodes.empty()) {
      CheckNodeForModuleSet(0, current_modules_vector, current_query_nodes,
                            scheduled_queries, starting_nodes,
                            supported_accelerator_bitstreams,
                            existing_modules_library);
    }
  } else {
    if (node_index + 1 != starting_nodes.size()) {
      CheckNodeForModuleSet(node_index + 1, current_modules_vector,
                            current_query_nodes, scheduled_queries,
                            starting_nodes, supported_accelerator_bitstreams,
                            existing_modules_library);
    }
  }
}

// Check if the desired module can be added to the existing combination
auto NodeScheduler::FindSuitableModuleCombination(
    query_scheduling_data::QueryNode* current_node,
    const std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    const query_scheduling_data::ConfigurableModulesVector&
        current_modules_vector,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<fpga_managing::operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library)
    -> query_scheduling_data::ConfigurableModulesVector {
  if (current_node->operation_type ==
      fpga_managing::operation_types::QueryOperationType::kPassThrough) {
    return current_modules_vector;
  }

  int current_position = FindMinPosition(current_node, current_query_nodes,
                                         current_modules_vector);

  auto find_iterator =
      existing_modules_library.find(current_node->operation_type);
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
    // Resource elastic requirements
    if (!module_parameters.empty()) {
      auto new_modules_vector = CheckModuleParameterSupport(
          module_parameters, current_modules_vector, module_position, 0,
          current_node->operation_type, current_module_possible_parameters,
          supported_accelerator_bitstreams);
      if (!new_modules_vector.empty()) {
        current_node->module_location = module_position + 1;
        return new_modules_vector;
      }
      // TODO(Kaspar): Make this nicer. A bit smelly
      // No resource elastic requirements
    } else {
      auto new_modules_vector =
          CreateNewModulesVector(current_node->operation_type, module_position,
                                 current_modules_vector, module_parameters);
      if (IsModuleSetSupported(new_modules_vector,
                               supported_accelerator_bitstreams)) {
        current_node->module_location = module_position + 1;
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
    const query_scheduling_data::ConfigurableModulesVector&
        current_modules_vector,
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
    const query_scheduling_data::ConfigurableModulesVector&
        current_modules_vector,
    const std::vector<int>& module_parameters)
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
    auto observed_node = required_node.lock();
    if (observed_node && !IsNodeIncluded(scheduled_nodes, *observed_node)) {
      return false;
    }
  }
  return true;
}

auto NodeScheduler::CreateReferenceVector(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        pointer_vector) -> std::vector<query_scheduling_data::QueryNode> {
  std::vector<query_scheduling_data::QueryNode> ref_vector;
  for (const auto& ptr : pointer_vector) {
    ref_vector.push_back(*ptr);
  }
  return ref_vector;
}

//// Find function for getting an iterator for a node which is available.
// auto NodeScheduler::FindNextAvailableNode(
//    const std::vector<query_scheduling_data::QueryNode>&
//    already_scheduled_nodes, std::vector<query_scheduling_data::QueryNode>&
//    starting_nodes)
//    -> std::vector<query_scheduling_data::QueryNode>::iterator& {
//  return std::find_if(starting_nodes.begin(), starting_nodes.end(),
//                      [&](
//                          const query_scheduling_data::QueryNode& leaf_node) {
//                        return IsNodeAvailable(already_scheduled_nodes,
//                                               leaf_node);
//                      });
//}
