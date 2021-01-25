#include "query_manager.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>

#include "accelerated_query_node.hpp"
#include "data_manager.hpp"
#include "fpga_manager.hpp"
#include "id_manager.hpp"
#include "memory_block_interface.hpp"
#include "node_scheduler.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "query_scheduling_data.hpp"
#include "stream_parameter_calculator.hpp"
#include "table_manager.hpp"

void QueryManager::CheckTableData(const TableData& expected_table,
                                  const TableData& resulting_table) {
  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    std::cout << expected_table.table_data_vector.size() /
                     TableManager::GetRecordSizeFromTable(expected_table)
              << std::endl;
    // DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    std::cout << resulting_table.table_data_vector.size() /
                     TableManager::GetRecordSizeFromTable(resulting_table)
              << std::endl;
    DataManager::PrintTableData(resulting_table);
  }
}

auto QueryManager::GetBitstreamFileFromQueryNode(
    const std::pair<query_scheduling_data::ConfigurableModulesVector,
                    std::vector<query_scheduling_data::QueryNode>>& query_node)
    -> std::string {
  auto bitstreams_iterator =
      query_scheduling_data::supported_accelerator_bitstreams.find(
          query_node.first);
  if (bitstreams_iterator !=
      query_scheduling_data::supported_accelerator_bitstreams.end()) {
    return bitstreams_iterator->second;
  } else {
    throw std::runtime_error("Unsopported set of modules!");
  }
}

auto QueryManager::GetModuleCountFromQueryNode(
    const std::pair<query_scheduling_data::ConfigurableModulesVector,
                    std::vector<query_scheduling_data::QueryNode>>& query_node)
    -> int {
  int module_count = 1;  // +1 for DMA module
  for (const auto& operation_module : query_node.first) {
    module_count++;
  }
  return module_count;
}

void QueryManager::RunQueries(
    std::vector<query_scheduling_data::QueryNode> starting_query_nodes) {
  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");
  MemoryManager memory_manager;
  FPGAManager fpga_manager(&memory_manager);

  std::queue<std::pair<query_scheduling_data::ConfigurableModulesVector,
                       std::vector<query_scheduling_data::QueryNode>>>
      query_node_runs_queue;

  NodeScheduler::FindAcceleratedQueryNodeSets(&query_node_runs_queue,
                                              std::move(starting_query_nodes));

  while (!query_node_runs_queue.empty()) {
    const auto executable_query_nodes = query_node_runs_queue.front();

    memory_manager.LoadBitstreamIfNew(
        GetBitstreamFileFromQueryNode(executable_query_nodes),
        GetModuleCountFromQueryNode(executable_query_nodes) *
            query_acceleration_constants::kModuleSize);

    IDManager id_manager;
    std::vector<std::vector<int>> output_ids;
    std::vector<std::vector<int>> input_ids;
    std::vector<std::vector<std::unique_ptr<MemoryBlockInterface>>>
        input_memory_blocks;
    std::vector<std::vector<std::unique_ptr<MemoryBlockInterface>>>
        output_memory_blocks;
    std::vector<TableData> expected_output_tables(16);
    std::vector<AcceleratedQueryNode> query_nodes;

    id_manager.AllocateStreamIDs(executable_query_nodes.second, input_ids,
                                 output_ids);

    for (int node_index = 0; node_index < executable_query_nodes.second.size();
         node_index++) {
      auto current_node = executable_query_nodes.second.at(node_index);
      // Allocate memory blocks
      std::vector<std::unique_ptr<MemoryBlockInterface>>
          allocated_input_memory_blocks;
      for (const auto& linked_node : current_node.previous_nodes) {
        if (!linked_node) {
          allocated_input_memory_blocks.push_back(
              memory_manager.GetAvailableMemoryBlock());
        } else {
          allocated_input_memory_blocks.push_back(nullptr);
        }
      }

      std::vector<std::unique_ptr<MemoryBlockInterface>>
          allocated_output_memory_blocks;
      for (const auto& linked_node : current_node.next_nodes) {
        if (!linked_node) {
          allocated_output_memory_blocks.push_back(
              memory_manager.GetAvailableMemoryBlock());
        } else {
          allocated_output_memory_blocks.push_back(nullptr);
        }
      }

      // Get parameters and write input to allocated blocks
      std::vector<StreamDataParameters> input_stream_parameters;
      TableManager::ReadInputTables(input_stream_parameters, data_manager,
                                    current_node.input_data_definition_files,
                                    input_ids[node_index],
                                    allocated_input_memory_blocks);

      std::vector<StreamDataParameters> output_stream_parameters;
      TableManager::ReadExpectedTables(
          output_stream_parameters, data_manager,
          current_node.output_data_definition_files, output_ids[node_index],
          allocated_output_memory_blocks, expected_output_tables);

      query_nodes.push_back({std::move(input_stream_parameters),
                             std::move(output_stream_parameters),
                             current_node.operation_type});

      // Keep memory blocks during the query execution
      input_memory_blocks.push_back(std::move(allocated_input_memory_blocks));
      output_memory_blocks.push_back(std::move(allocated_output_memory_blocks));
    }

    // Run query
    fpga_manager.SetupQueryAcceleration(query_nodes);
    std::cout << "Running query!" << std::endl;
    auto result_sizes = fpga_manager.RunQueryAcceleration();
    std::cout << "Query done!" << std::endl;

    // Check results & free memory
    std::vector<TableData> output_tables = expected_output_tables;
    for (int node_index = 0; node_index < query_nodes.size(); node_index++) {
      TableManager::ReadResultTables(query_nodes[node_index].output_streams,
                                     output_tables, result_sizes,
                                     output_memory_blocks[node_index]);
      for (int stream_index = 0; stream_index < output_ids[node_index].size();
           stream_index++) {
        if (output_memory_blocks[node_index][stream_index]) {
          std::cout << "Result has "
                    << result_sizes[output_ids[node_index][stream_index]]
                    << " rows!" << std::endl;

          CheckTableData(
              expected_output_tables[output_ids[node_index][stream_index]],
              output_tables[output_ids[node_index][stream_index]]);
        }
      }

      // Free all memory for now.
      for (auto& memory_pointer : input_memory_blocks[node_index]) {
        if (memory_pointer) {
          memory_manager.FreeMemoryBlock(std::move(memory_pointer));
        }
      }
      for (auto& memory_pointer : output_memory_blocks[node_index]) {
        if (memory_pointer) {
          memory_manager.FreeMemoryBlock(std::move(memory_pointer));
        }
      }
    }

    query_node_runs_queue.pop();
  }
}