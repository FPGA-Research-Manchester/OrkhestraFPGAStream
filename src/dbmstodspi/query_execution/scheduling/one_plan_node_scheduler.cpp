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

#include "one_plan_node_scheduler.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "query_scheduling_data.hpp"
#include "run_linker.hpp"
#include "util.hpp"

using orkhestrafs::dbmstodspi::OnePlanNodeScheduler;
using orkhestrafs::dbmstodspi::RunLinker;
using orkhestrafs::dbmstodspi::query_acceleration_constants::kIOStreamParamDefs;
using orkhestrafs::dbmstodspi::util::CreateReferenceVector;

auto OnePlanNodeScheduler::FindAcceleratedQueryNodeSets(
    std::vector<std::shared_ptr<QueryNode>> starting_nodes,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams,
    const std::map<QueryOperationType, std::vector<std::vector<int>>>&
        existing_modules_library,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes)
    -> std::queue<std::pair<ConfigurableModulesVector,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  std::queue<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>
      query_node_runs_queue;

  std::vector<std::shared_ptr<QueryNode>> scheduled_queries;

  // Not checking for cycles nor for unsupported opperations
  while (!starting_nodes.empty()) {
    int node_index = 0;

    ConfigurableModulesVector current_set;
    std::vector<std::shared_ptr<QueryNode>> current_query_nodes;

    CheckNodeForModuleSet(node_index, current_set, current_query_nodes,
                          scheduled_queries, starting_nodes,
                          supported_accelerator_bitstreams,
                          existing_modules_library);

    if (current_query_nodes.empty()) {
      throw std::runtime_error("Failed to schedule!");
    }

    query_node_runs_queue.push({current_set, current_query_nodes});
  }
  return RunLinker::LinkPeripheralNodesFromGivenRuns(query_node_runs_queue,
                                                     linked_nodes);
}

// Function to find the minimum position for a node such that all the
// prerequisites are met
auto OnePlanNodeScheduler::FindMinPosition(
    const QueryNode* current_node,

    const std::vector<QueryNode>& current_query_nodes,
    const ConfigurableModulesVector& current_modules_vector) -> int {
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
          observed_node->operation_type != QueryOperationType::kPassThrough) {
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
            [&](const QueryOperation& operation) {
              return operation.operation_type == observed_node->operation_type;
            });
        if (current_modules_iterator == current_modules_vector.end()) {
          throw std::runtime_error("Something went wrong with scheduling!");
        }
        int current_position_index =
            current_modules_iterator - current_modules_vector.begin() + 1;
        if (current_position_index > min_position_index) {
          min_position_index = current_position_index;
        }
      }
    }
  }
  return min_position_index;
}

auto OnePlanNodeScheduler::IsProjectionOperationDefined(
    const QueryNode* current_node, const QueryNode* previous_node,
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
void OnePlanNodeScheduler::CheckNodeForModuleSet(
    int previous_node_index, ConfigurableModulesVector& current_modules_vector,
    std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
    std::vector<std::shared_ptr<QueryNode>>& scheduled_queries,
    std::vector<std::shared_ptr<QueryNode>>& starting_nodes,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams,
    const std::map<QueryOperationType, std::vector<std::vector<int>>>&
        existing_modules_library) {
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
auto OnePlanNodeScheduler::FindSuitableModuleCombination(
    QueryNode* current_node, const std::vector<QueryNode>& current_query_nodes,
    const ConfigurableModulesVector& current_modules_vector,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams,
    const std::map<QueryOperationType, std::vector<std::vector<int>>>&
        existing_modules_library) -> ConfigurableModulesVector {
  if (current_node->operation_type == QueryOperationType::kPassThrough) {
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
        current_node->module_locations.push_back(module_position + 1);
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
        current_node->module_locations.push_back(module_position + 1);
        return new_modules_vector;
      }
    }
  }
  return {};
}

// Recursive method to go through all the module parameters to find the first
// supported one
auto OnePlanNodeScheduler::CheckModuleParameterSupport(
    std::vector<int> module_parameters,
    const ConfigurableModulesVector& current_modules_vector,
    int module_position, int parameter_option_index,
    QueryOperationType query_operation,
    std::vector<std::vector<int>> current_module_possible_parameters,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams) -> ConfigurableModulesVector {
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
auto OnePlanNodeScheduler::CreateNewModulesVector(
    QueryOperationType query_operation, int current_position,
    const ConfigurableModulesVector& current_modules_vector,
    const std::vector<int>& module_parameters) -> ConfigurableModulesVector {
  if (query_operation == QueryOperationType::kPassThrough) {
    return current_modules_vector;
  }
  if (current_modules_vector.empty()) {
    return {{query_operation, module_parameters}};
  }
  if (current_position == current_modules_vector.size()) {
    ConfigurableModulesVector temp_current_modules_vector =
        current_modules_vector;
    temp_current_modules_vector.emplace_back(query_operation,
                                             module_parameters);
    return temp_current_modules_vector;
  }
  ConfigurableModulesVector temp_current_modules_vector(
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
auto OnePlanNodeScheduler::IsModuleSetSupported(
    const ConfigurableModulesVector& module_set,
    const std::map<ConfigurableModulesVector, std::string>&
        supported_accelerator_bitstreams) -> bool {
  return supported_accelerator_bitstreams.find(module_set) !=
         supported_accelerator_bitstreams.end();
}

// Check if the given node is in the given vector.
auto OnePlanNodeScheduler::IsNodeIncluded(
    const std::vector<QueryNode>& node_vector, const QueryNode& searched_node)
    -> bool {
  return std::any_of(node_vector.begin(), node_vector.end(),
                     [&](const QueryNode& scheduled_node) {
                       return scheduled_node == searched_node;
                     });
}

// Check if all required nodes have been scheduled already
auto OnePlanNodeScheduler::IsNodeAvailable(
    const std::vector<QueryNode>& scheduled_nodes,
    const QueryNode& current_node) -> bool {
  for (const auto& required_node : current_node.previous_nodes) {
    auto observed_node = required_node.lock();
    if (observed_node && !IsNodeIncluded(scheduled_nodes, *observed_node)) {
      return false;
    }
  }
  return true;
}

auto OnePlanNodeScheduler::FindNextNodeLocation(
    const std::vector<std::shared_ptr<QueryNode>>& next_nodes,
    const QueryNode* next_node) -> int {
  for (int next_node_index = 0; next_node_index < next_nodes.size();
       next_node_index++) {
    if (next_nodes[next_node_index].get() == next_node) {
      return next_node_index;
    }
  }
  throw std::runtime_error("No node found!");
}
