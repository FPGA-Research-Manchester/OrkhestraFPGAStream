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

#include "node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "util.hpp"

using namespace dbmstodspi::query_managing;
using namespace dbmstodspi::fpga_managing;
using dbmstodspi::query_managing::query_scheduling_data::kIOStreamParamDefs;
using dbmstodspi::util::CreateReferenceVector;

auto NodeScheduler::FindAcceleratedQueryNodeSets(
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        starting_nodes,
    const std::map<query_scheduling_data::ConfigurableModulesVector,
                   std::string>& supported_accelerator_bitstreams,
    const std::map<operation_types::QueryOperationType,
                   std::vector<std::vector<int>>>& existing_modules_library,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes)
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

    CheckExternalLinks(current_query_nodes, linked_nodes);

    query_node_runs_queue.push({current_set, current_query_nodes});
  }
  return query_node_runs_queue;
}

// Method to remove next or previous nodes from a node once it has been
// scheduled
void NodeScheduler::CheckExternalLinks(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        current_query_nodes,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes) {
  for (const auto& node : current_query_nodes) {
    std::map<int, std::vector<std::pair<std::string, int>>> target_maps;
    for (int next_node_index = 0; next_node_index < node->next_nodes.size();
         next_node_index++) {
      std::vector<std::pair<std::string, int>> targets;
      if (!node->next_nodes[next_node_index]) {
        if (node->output_data_definition_files[next_node_index].empty()) {
          node->output_data_definition_files[next_node_index] =
              node->node_name + "_" + std::to_string(next_node_index) + ".csv";
        }
      } else if (IsNodeMissingFromTheVector(node->next_nodes[next_node_index],
                                            current_query_nodes)) {
        int current_node_location = FindPreviousNodeLocation(
            node->next_nodes[next_node_index]->previous_nodes, node);
        auto current_filename =
            node->output_data_definition_files[next_node_index];
        if (current_filename.empty() &&
            ReuseMemory(*node, *node->next_nodes[next_node_index])) {
          targets.emplace_back(node->next_nodes[next_node_index]->node_name,
                               current_node_location);
        } else {
          if (current_filename.empty()) {
            current_filename = node->node_name + "_" +
                               std::to_string(next_node_index) + ".csv";
            node->output_data_definition_files[next_node_index] =
                current_filename;
          }
          node->next_nodes[next_node_index]
              ->input_data_definition_files[current_node_location] =
              current_filename;
          node->next_nodes[next_node_index] = nullptr;
        }
      }
      if (!targets.empty()) {
        target_maps.insert({next_node_index, targets});
      }
    }
    for (auto& previous_node : node->previous_nodes) {
      auto observed_node = previous_node.lock();
      if (IsNodeMissingFromTheVector(observed_node, current_query_nodes)) {
        previous_node = std::weak_ptr<query_scheduling_data::QueryNode>();
      }
    }
    if (!target_maps.empty()) {
      linked_nodes.insert({node->node_name, target_maps});
    }
  }
}

auto NodeScheduler::ReuseMemory(
    const query_scheduling_data::QueryNode& /*source_node*/,
    const query_scheduling_data::QueryNode& /*target_node*/) -> bool {
  // TODO(Kaspar): needs to consider the memory capabilities
  return true;
}

auto NodeScheduler::IsNodeMissingFromTheVector(
    const std::shared_ptr<
        dbmstodspi::query_managing::query_scheduling_data::QueryNode>&
        linked_node,
    const std::vector<std::shared_ptr<
        dbmstodspi::query_managing::query_scheduling_data::QueryNode>>&
        current_query_nodes) -> bool {
  return linked_node &&
         std::find(current_query_nodes.begin(), current_query_nodes.end(),
                   linked_node) == current_query_nodes.end();
}

// Function to find the minimum position for a node such that all the
// prerequisites are met
auto NodeScheduler::FindMinPosition(
    const query_scheduling_data::QueryNode* current_node,

    const std::vector<query_scheduling_data::QueryNode>& current_query_nodes,
    const query_scheduling_data::ConfigurableModulesVector&
        current_modules_vector) -> int {
  int min_position_index = 0;
  for (int previous_node_index = 0;
       previous_node_index < current_node->previous_nodes.size();
       previous_node_index++) {
    auto observed_node =
        current_node->previous_nodes[previous_node_index].lock();
    if (observed_node) {
      auto current_nodes_iterator =
          std::find(current_query_nodes.begin(), current_query_nodes.end(),
                    *observed_node);
      if (current_nodes_iterator != current_query_nodes.end() &&
          observed_node->operation_type !=
              fpga_managing::operation_types::QueryOperationType::
                  kPassThrough) {
        // Assuming that the projection vector exists.
        int current_node_index =
            FindNextNodeLocation(observed_node->next_nodes, current_node);
        if (IsProjectionOperationDefined(current_node, observed_node.get(),
                                         previous_node_index,
                                         current_node_index) ||
            observed_node->is_checked[previous_node_index]) {
          return -1;
        }

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

auto NodeScheduler::IsProjectionOperationDefined(
    const query_scheduling_data::QueryNode* current_node,
    const query_scheduling_data::QueryNode* previous_node,
    int previous_node_index, int current_node_index) -> bool {
  return !previous_node->operation_parameters
              .output_stream_parameters
                  [current_node_index * kIOStreamParamDefs.kStreamParamCount +
                   kIOStreamParamDefs.kProjectionOffset]
              .empty() ||
         !current_node->operation_parameters
              .input_stream_parameters
                  [previous_node_index * kIOStreamParamDefs.kStreamParamCount +
                   kIOStreamParamDefs.kProjectionOffset]
              .empty();
}

// Check recursively if the given node can be added to the set of nodes to be
// scheduled
void NodeScheduler::CheckNodeForModuleSet(
    int previous_node_index,
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
  auto current_node = starting_nodes[previous_node_index];

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

    starting_nodes.erase(starting_nodes.begin() + previous_node_index);

    if (!starting_nodes.empty()) {
      CheckNodeForModuleSet(0, current_modules_vector, current_query_nodes,
                            scheduled_queries, starting_nodes,
                            supported_accelerator_bitstreams,
                            existing_modules_library);
    }
  } else {
    if (previous_node_index + 1 != starting_nodes.size()) {
      CheckNodeForModuleSet(previous_node_index + 1, current_modules_vector,
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
  if (current_position == -1) {
    return {};
  }

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
auto NodeScheduler::CheckModuleParameterSupport(
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

auto NodeScheduler::FindNextNodeLocation(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        next_nodes,
    const query_scheduling_data::QueryNode* next_node) -> int {
  for (int next_node_index = 0; next_node_index < next_nodes.size();
       next_node_index++) {
    if (next_nodes[next_node_index].get() == next_node) {
      return next_node_index;
    }
  }
  throw std::runtime_error("No node found!");
}

auto NodeScheduler::FindPreviousNodeLocation(
    const std::vector<std::weak_ptr<query_scheduling_data::QueryNode>>&
        previous_nodes,
    const std::shared_ptr<query_scheduling_data::QueryNode>& previous_node)
    -> int {
  for (int previous_node_index = 0; previous_node_index < previous_nodes.size();
       previous_node_index++) {
    auto observed_node = previous_nodes[previous_node_index].lock();
    if (observed_node == previous_node) {
      return previous_node_index;
    }
  }
  throw std::runtime_error("No node found!");
}
