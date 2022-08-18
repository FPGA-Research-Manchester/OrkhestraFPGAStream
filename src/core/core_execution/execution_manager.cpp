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

#include "execution_manager.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

#include "query_scheduling_helper.hpp"

using orkhestrafs::core::core_execution::ExecutionManager;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

void ExecutionManager::SetFinishedFlag() { busy_flag_ = false; }

void ExecutionManager::UpdateAvailableNodesGraph() {
  current_available_node_pointers_.clear();
  if (!processed_nodes_.empty()) {
    unscheduled_graph_->DeleteNodes(processed_nodes_);
    processed_nodes_.clear();
  }

  current_available_node_pointers_ = unscheduled_graph_->GetRootNodesPtrs();
  current_available_node_names_.clear();
  for (const auto& node : current_available_node_pointers_) {
    current_available_node_names_.insert(node->node_name);
  }
  RemoveUnusedTables(current_tables_metadata_,
                     unscheduled_graph_->GetAllNodesPtrs());
  table_counter_.clear();
  InitialiseTables(current_tables_metadata_, current_available_node_pointers_,
                   query_manager_.get(), data_manager_.get());
  current_query_graph_.clear();
  SetupTableDependencies(unscheduled_graph_->GetAllNodesPtrs(), blocked_nodes_, table_counter_);
  SetupSchedulingGraphAndConstrainedNodes(
      unscheduled_graph_->GetAllNodesPtrs(), current_query_graph_,
      *accelerator_library_, nodes_constrained_to_first_,
      current_tables_metadata_);
}

void ExecutionManager::SetupTableDependencies(
    const std::vector<QueryNode*>& all_nodes,
    std::unordered_set<std::string>& blocked_nodes,
    std::unordered_map<std::string, int>& table_counter) {
  blocked_nodes.clear();
  std::unordered_set<std::string> output_tables;
  for (const auto& node : all_nodes) {
    for (const auto& output : node->given_output_data_definition_files) {
      if (!output.empty()) {
        output_tables.insert(output);
      }
    }
  }
  std::unordered_set<QueryNode*> nodes_to_check;
  // we want to block all nodes that have an input that is the output of something else
  // But nodes that have this condition and are linked do not get blocked!
  for (const auto& node : all_nodes) {
    for (int i = 0; i < node->given_input_data_definition_files.size(); i++) {
      const auto& input_table = node->given_input_data_definition_files.at(i);
      const auto& input_node = node->previous_nodes.at(i);
      if (!input_table.empty() &&
          output_tables.find(input_table) != output_tables.end()) {
        if (!input_node ||
            std::none_of(
                input_node->given_output_data_definition_files.begin(),
                input_node->given_output_data_definition_files.end(),
                [&](auto table_name) { return input_table == table_name; })) {
          blocked_nodes.insert(node->node_name);
          nodes_to_check.insert(node);
          
          
        }
      }
    }
  }
  // To prevent blocked tables from being deleted
  while (!nodes_to_check.empty()) {
    auto cur_node = *nodes_to_check.begin();
    nodes_to_check.erase(nodes_to_check.begin());
    for (const auto& input_table :
         cur_node->given_input_data_definition_files) {
      if (!input_table.empty()) {
        table_counter.insert({input_table, 1});
      }
    }
    for (const auto& output_node : cur_node->next_nodes) {
      if (output_node) {
        nodes_to_check.insert(output_node);
      }
    }
  }

  // If we want to be more precise
  /*std::unordered_map<std::string, std::unordered_set<std::string>>
      tables_to_nodes_used_map;
  for (const auto& node : all_nodes) {
    for (const auto& input : node->given_input_data_definition_files) {
      if (!input.empty()) {
        std::unordered_set<std::string> new_set = {node->node_name};
        if (const auto& [it, inserted] =
                tables_to_nodes_used_map.try_emplace(input, new_set);
            !inserted) {
          it->second.merge(new_set);
        }
      }
    }
  }
  for (const auto& node : all_nodes) {
    for (int i = 0; i < node->given_output_data_definition_files.size(); i++) {
      const auto& output = node->given_output_data_definition_files.at(i);
      if (!output.empty()) {
        if (node->is_checked.at(i) &&
            input_tables.find(output) != input_tables.end() &&
            tables_metadata.find(output) != tables_metadata.end()) {
          tables_metadata.at(output).is_finished = false;
        }
      }
    }
  }*/
  /*std::unordered_set<std::string> input_tables;
  for (const auto& node : all_nodes) {
    for (const auto& input : node->given_input_data_definition_files) {
      if (!input.empty()) {
        input_tables.insert(input);
      }
    }
  }
  for (const auto& node : all_nodes) {
    for (int i = 0; i < node->given_output_data_definition_files.size(); i++) {
      const auto& output = node->given_output_data_definition_files.at(i);
      if (!output.empty()) {
        if (node->is_checked.at(i) &&
            input_tables.find(output) != input_tables.end() &&
            tables_metadata.find(output) != tables_metadata.end()) {
          tables_metadata.at(output).is_finished = false;
        }
      }
    }
  }*/
}

void ExecutionManager::RemoveUnusedTables(
    std::map<std::string, TableMetadata>& tables_metadata,
    const std::vector<QueryNode*>& all_nodes) {
  // TODO(Kaspar): Possibly reserve some space beforehand.
  std::unordered_set<std::string> required_tables;
  std::unordered_set<std::string> output_tables;
  for (const auto& node : all_nodes) {
    for (const auto& input : node->given_input_data_definition_files) {
      if (!input.empty()) {
        required_tables.insert(input);
      }
    }
    for (const auto& output : node->given_output_data_definition_files) {
      if (!output.empty()) {
        required_tables.insert(output);
        output_tables.insert(output);
      }
    }
  }
  std::vector<std::string> tables_to_delete;
  auto missing_tables = required_tables;
  for (const auto& [table_name, data] : tables_metadata) {
    missing_tables.erase(table_name);
    if (required_tables.find(table_name) == required_tables.end()) {
      tables_to_delete.push_back(table_name);
    }
  }
  for (const auto& output : output_tables) {
    missing_tables.erase(output);
  }
  if (!missing_tables.empty()) {
    throw std::runtime_error(
        "There are tables required that are not available!");
  }
  for (const auto& table_to_delete : tables_to_delete) {
    tables_metadata.erase(table_to_delete);
  }
}

// Initially only input nodes have input tables defined.
// This method names all the rest of the tables.
void ExecutionManager::InitialiseTables(
    std::map<std::string, TableMetadata>& tables_metadata,
    std::vector<QueryNode*> current_available_node_pointers,
    const QueryManagerInterface* query_manager,
    const DataManagerInterface* data_manager) {
  // Setup new unintialised tables
  while (!current_available_node_pointers.empty()) {
    auto* current_node = current_available_node_pointers.back();
    current_available_node_pointers.pop_back();
    if (std::any_of(current_node->given_input_data_definition_files.begin(),
                    current_node->given_input_data_definition_files.end(),
                    [](const auto& filename) { return filename.empty(); })) {
      throw std::runtime_error("Table initialisation has an empty table!");
    }
    for (int output_stream_id = 0;
         output_stream_id < current_node->next_nodes.size();
         output_stream_id++) {
      auto& table_name =
          current_node->given_output_data_definition_files.at(output_stream_id);
      if (table_name.empty()) {
        table_name =
            current_node->node_name + "_" + std::to_string(output_stream_id);
        TableMetadata new_data;
        new_data.record_size = query_manager->GetRecordSizeFromParameters(
            data_manager,
            current_node->given_operation_parameters.output_stream_parameters,
            output_stream_id);
        // Hardcoded for benchmarking
        //new_data.record_size = 10;
        new_data.record_count = -1;
        tables_metadata.insert({table_name, new_data});
      }

      const auto& output_node = current_node->next_nodes.at(output_stream_id);
      if (output_node) {
        int index = GetCurrentNodeIndexFromNextNode(current_node, output_node);
        if (output_node->given_input_data_definition_files.at(index).empty()) {
          output_node->given_input_data_definition_files[index] = table_name;

          if (std::all_of(
                  output_node->given_input_data_definition_files.begin(),
                  output_node->given_input_data_definition_files.end(),
                  [](const auto& filename) { return !filename.empty(); })) {
            current_available_node_pointers.push_back(output_node);
          }
        }
        // Else the table names have been already given to the output node.
      }
    }
  }
}

auto ExecutionManager::GetCurrentNodeIndexFromNextNode(QueryNode* current_node,
                                                       QueryNode* next_node)
    -> int {
  for (int i = 0; i < next_node->previous_nodes.size(); i++) {
    if (next_node->previous_nodes.at(i) == current_node) {
      return i;
    }
  }
  throw std::runtime_error("Node not found!");
}

void ExecutionManager::Execute(
    std::unique_ptr<ExecutionPlanGraphInterface> execution_graph) {
  unscheduled_graph_ = std::move(execution_graph);
  auto begin = std::chrono::steady_clock::now();
  busy_flag_ = true;
  while (busy_flag_) {
    auto new_state = current_state_->Execute(this);
    if (new_state) {
      current_state_ = std::move(new_state);
    }
  }
  auto end = std::chrono::steady_clock::now();
  auto data = query_manager_->GetData();
  long total_execution =
      std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
          .count();
  long data_size = data[0];
  long scheduling = data[3];
  long init_config = data[1];
  long config = config_time_;
  long initialisation = data[2];
  long system =
      total_execution - scheduling - init_config - config - initialisation;
  long actual_execution = total_execution - init_config;
  std::cout << "ACTUAL_EXECUTION: " << actual_execution << std::endl;

  /*std::cout << "CONFIGURATION: " << config << std::endl;
  std::cout << "SCHEDULING: " << scheduling << std::endl;
  std::cout << "INITIALISATION: " << initialisation << std::endl;
  std::cout << "SYSTEM: " << system
            << std::endl;
  std::cout << "EXECUTION: " << (data_size / 4659.61402505057)
            << std::endl;*/
  
  /*std::cout << "STATIC: "
            << ((data_size / 4659.61402505057) + initialisation)
            << std::endl;*/

  //std::cout << "DATA_STREAMED: " << data_size << std::endl;
  /*std::cout << "TOTAL EXECUTION RUNTIME: "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;*/
  /*Log(LogLevel::kInfo,
      "Overall time = " +
          to_string(
              chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");*/
}

auto ExecutionManager::IsUnscheduledNodesGraphEmpty() -> bool {
  return unscheduled_graph_->IsEmpty();
}
auto ExecutionManager::IsARunScheduled() -> bool {
  return !query_node_runs_queue_.empty();
}
auto ExecutionManager::IsRunReadyForExecution() -> bool {
  return !query_nodes_.empty();
}

void ExecutionManager::ScheduleUnscheduledNodes() {
  query_node_runs_queue_ = query_manager_->ScheduleNextSetOfNodes(
      current_available_node_pointers_, nodes_constrained_to_first_,
      current_available_node_names_, current_query_graph_,
      current_tables_metadata_, *accelerator_library_, config_, *scheduler_,
      current_configuration_, processed_nodes_, table_counter_, blocked_nodes_);
}
void ExecutionManager::BenchmarkScheduleUnscheduledNodes() {
  query_manager_->BenchmarkScheduling(
      nodes_constrained_to_first_, current_available_node_names_,
      processed_nodes_, current_query_graph_, current_tables_metadata_,
      *accelerator_library_, config_, *scheduler_, current_configuration_, blocked_nodes_);
}
auto ExecutionManager::IsBenchmarkDone() -> bool {
  return current_available_node_names_.empty();
}

void ExecutionManager::SetupNextRunData() {
  /*ConfigurableModulesVector next_set_of_operators;*/

  // const auto first = query_node_runs_queue_.front();  // getting the first
  // query_node_runs_queue_.pop();                       // removing him
  // query_node_runs_queue_.push(first);

  /*auto next_run = query_node_runs_queue_.front().first;
  for (auto& module_index : next_run) {
    next_set_of_operators.emplace_back(
        module_index.operation_type,
        config_.pr_hw_library.at(module_index.operation_type)
            .bitstream_map.at(module_index.bitstream)
            .capacity);
  }*/

  // if (config_.accelerator_library.find(next_set_of_operators) ==
  //    config_.accelerator_library.end()) {
  //  // for (const auto& thing : query_node_runs_queue_.front().first) {
  //  //  auto op_type = thing.operation_type;
  //  //  auto module_params = thing.resource_elasticity_data;
  //  //}
  //  throw std::runtime_error("Bitstream not found!");
  //}

  // Old
  /*query_manager_->LoadNextBitstreamIfNew(
      memory_manager_.get(),
      config_.accelerator_library.at(query_node_runs_queue_.front().first),
      config_);*/
  /*query_manager_->LoadInitialStaticBitstream(memory_manager_.get());
  query_manager_->LoadEmptyRoutingPRRegion(memory_manager_.get(),
                                           *accelerator_library_.get());*/
  auto [bitstreams_to_load, empty_modules] =
      query_manager_->GetPRBitstreamsToLoadWithPassthroughModules(
          current_configuration_, query_node_runs_queue_.front().first, current_routing_);
  query_manager_->LoadPRBitstreams(memory_manager_.get(), bitstreams_to_load,
                                   *accelerator_library_);
  config_time_ += query_manager_->LoadPRBitstreams(
      memory_manager_.get(), bitstreams_to_load,
                                   *accelerator_library_);
  /*query_manager_->LoadPRBitstreams(memory_manager_.get(), bitstreams_to_load,
   *accelerator_library_.get());*/
  // Debugging
  // std::vector<std::pair<QueryOperationType, bool>> empty_modules;
  /*query_manager_->LoadEmptyRoutingPRRegion(memory_manager_.get(),
   *accelerator_library_.get());*/
  /*query_manager_->LoadPRBitstreams(memory_manager_.get(),config_.debug_forced_pr_bitstreams,
   *accelerator_library_.get()); exit(0);*/
  auto next_scheduled_run_nodes = PopNextScheduledRun();
  scheduled_node_names_.clear();
  for (const auto& node : next_scheduled_run_nodes) {
    scheduled_node_names_.push_back(node->node_name);
  }

  /*for (const auto& node_name : scheduled_node_names_) {
    std::cout << node_name << " ";
  }*/
  /*std::cout << std::endl;
  for (const auto& [op, used] : empty_modules) {
    if (op == QueryOperationType::kAddition) {
      std::cout << "Add: " << (used ? "true"
                                   : "false")
                                         << ";";
    } else if (op == QueryOperationType::kAggregationSum) {
      std::cout << "Sum: " << (used ? "true" : "false") << ";";
    } else if (op == QueryOperationType::kFilter) {
      std::cout << "Filter: " << (used ? "true" : "false") << ";";
    } else if (op == QueryOperationType::kJoin) {
      std::cout << "Join: " << (used ? "true" : "false") << ";";
    } else if (op == QueryOperationType::kLinearSort) {
      std::cout << "LinSort: " << (used ? "true" : "false") << ";";
    } else if (op == QueryOperationType::kMergeSort) {
      std::cout << "MergeSort: " << (used ? "true" : "false") << ";";
    } else if (op == QueryOperationType::kMultiplication) {
      std::cout << "Mul: " << (used ? "true" : "false") << ";";
    }
  }*/

  // query_manager_->MeasureBitstreamConfigurationSpeed(config_.pr_hw_library,
  //                                                   memory_manager_.get());

  auto execution_nodes_and_result_params =
      query_manager_->SetupAccelerationNodesForExecution(
          data_manager_.get(), memory_manager_.get(),
          accelerator_library_.get(), next_scheduled_run_nodes,
          current_tables_metadata_, table_memory_blocks_, table_counter_);
  query_nodes_ = std::move(execution_nodes_and_result_params.first);
  for (int module_pos = 0; module_pos < empty_modules.size(); module_pos++) {
    if (empty_modules.at(module_pos).second) {
      query_nodes_.insert(
          query_nodes_.begin() + module_pos,
          accelerator_library_->GetEmptyModuleNode(
              empty_modules.at(module_pos).first, module_pos + 1));
    } else {
      query_nodes_.at(module_pos).operation_module_location = module_pos + 1;
    }
  }
  result_parameters_ = std::move(execution_nodes_and_result_params.second);
}
void ExecutionManager::ExecuteAndProcessResults() {
  query_manager_->ExecuteAndProcessResults(
      memory_manager_.get(), fpga_manager_.get(), data_manager_.get(),
      table_memory_blocks_, result_parameters_, query_nodes_,
      current_tables_metadata_, table_counter_, config_.execution_timeout);
  // TODO(Kaspar): Remove this
  scheduled_node_names_.clear();
}
// auto ExecutionManager::IsRunValid() -> bool {
//  return query_manager_->IsRunValid(query_nodes_);
//}

auto ExecutionManager::PopNextScheduledRun() -> std::vector<QueryNode*> {
  const auto executable_query_nodes = query_node_runs_queue_.front().second;
  query_node_runs_queue_.pop();
  return executable_query_nodes;
}

void ExecutionManager::PrintCurrentStats() {
  query_manager_->PrintBenchmarkStats();
}

void ExecutionManager::SetupSchedulingData(bool setup_bitstreams) {
  config_time_ = 0;
  current_tables_metadata_ = config_.initial_all_tables_metadata;
  if (setup_bitstreams) {
    query_manager_->LoadInitialStaticBitstream(memory_manager_.get());
    // TODO: Remove the hardcoded aspect of this!
    for (int i = 0; i < 31; i++) {
      current_routing_.push_back("RT");
    }
  }
}

void ExecutionManager::SetupSchedulingGraphAndConstrainedNodes(
    const std::vector<QueryNode*>& all_query_nodes,
    std::unordered_map<std::string, SchedulingQueryNode>&
        current_scheduling_graph,
    AcceleratorLibraryInterface& hw_library,
    std::unordered_set<std::string>& constrained_nodes_vector,
    const std::map<std::string, TableMetadata>& tables_metadata) {
  for (const auto& node : all_query_nodes) {
    AddSchedulingNodeToGraph(node, current_scheduling_graph, hw_library);

    AddSavedNodesToConstrainedList(node, constrained_nodes_vector);
    AddFirstModuleNodesToConstrainedList(node, constrained_nodes_vector,
                                         hw_library);
  }
  AddSplittingNodesToConstrainedList(current_scheduling_graph,
                                     constrained_nodes_vector);
}

void ExecutionManager::AddSavedNodesToConstrainedList(
    QueryNode* const& node,
    std::unordered_set<std::string>& constrained_nodes) {
  for (int node_index = 0; node_index < node->is_checked.size(); node_index++) {
    if (node->is_checked[node_index] &&
        std::any_of(
            node->next_nodes.begin(), node->next_nodes.end(),
            [](const auto& next_node) { return next_node != nullptr; })) {
      auto& next_node = node->next_nodes[node_index];
      if (next_node != nullptr) {
        auto& constrained_node_name = node->next_nodes[node_index]->node_name;
        if (constrained_nodes.find(constrained_node_name) ==
            constrained_nodes.end()) {
          constrained_nodes.insert(constrained_node_name);
        }
      }
    }
  }
}

void ExecutionManager::AddSchedulingNodeToGraph(
    QueryNode* const& node,
    std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
    AcceleratorLibraryInterface& accelerator_library) {
  SchedulingQueryNode current_node;
  for (const auto& next_node : node->next_nodes) {
    if (next_node) {
      current_node.after_nodes.push_back(next_node->node_name);
    } else {
      current_node.after_nodes.emplace_back("");
    }
  }
  for (const auto& previous_node_ptr : node->previous_nodes) {
    if (previous_node_ptr) {
      current_node.before_nodes.emplace_back(
          previous_node_ptr->node_name,
          QuerySchedulingHelper::FindNodePtrIndex(node, previous_node_ptr));
    } else {
      current_node.before_nodes.emplace_back("", -1);
    }
  }
  for (const auto& input_files : node->given_input_data_definition_files) {
    current_node.data_tables.push_back(input_files);
  }
  current_node.operation = node->operation_type;
  current_node.capacity = accelerator_library.GetNodeCapacity(
      node->operation_type,
      node->given_operation_parameters.operation_parameters);
  current_node.node_ptr = node;
  scheduling_graph.insert({node->node_name, current_node});
}

void ExecutionManager::AddFirstModuleNodesToConstrainedList(
    QueryNode* const& node, std::unordered_set<std::string>& constrained_nodes,
    AcceleratorLibraryInterface& accelerator_library) {
  if (accelerator_library.IsNodeConstrainedToFirstInPipeline(
          node->operation_type)) {
    constrained_nodes.insert(node->node_name);
  };
}

// This check is looking at all the nodes if there are multiple identical before
// streams.
// TODO(Kaspar): Splitting nodes aren't supported at the moment anyway:
// after_nodes needs to be a vector of vectors!
void ExecutionManager::AddSplittingNodesToConstrainedList(
    std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
    std::unordered_set<std::string>& constrained_nodes) {
  // THere are no splitting nodes at the moment
  /*std::vector<std::pair<std::string, int>> all_stream_dependencies;
  std::vector<std::pair<std::string, int>> splitting_streams;*/

  /*for (const auto& [node_name, node_parameters] : scheduling_graph) {
    for (const auto& previous_node_stream : node_parameters.before_nodes) {
      if (previous_node_stream.second != -1) {
        if (std::find(all_stream_dependencies.begin(),
                      all_stream_dependencies.end(),
                      previous_node_stream) == all_stream_dependencies.end()) {
          all_stream_dependencies.push_back(previous_node_stream);
        } else if (std::find(splitting_streams.begin(), splitting_streams.end(),
                             previous_node_stream) == splitting_streams.end()) {
          splitting_streams.push_back(previous_node_stream);
        }
      }
    }
  }
  for (const auto& splitting_node_stream : splitting_streams) {
    for (const auto& potentially_constrained_node_name :
         scheduling_graph.at(splitting_node_stream.first).after_nodes) {
      if (!potentially_constrained_node_name.empty()) {
        auto before_nodes_vector =
            scheduling_graph.at(potentially_constrained_node_name).before_nodes;
        if (std::find(before_nodes_vector.begin(), before_nodes_vector.end(),
                      splitting_node_stream) != before_nodes_vector.end() &&
            std::find(constrained_nodes.begin(), constrained_nodes.end(),
                      potentially_constrained_node_name) ==
                constrained_nodes.end()) {
          constrained_nodes.insert(potentially_constrained_node_name);
        }
      }
    }
  }*/
}
