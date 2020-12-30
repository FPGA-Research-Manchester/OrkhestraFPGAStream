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
    const std::pair<query_scheduling_data::ConfigurableModuleSet,
                    std::vector<query_scheduling_data::QueryNode>>& query_node)
    -> std::string {
  auto bitstreams_iterator =
      query_scheduling_data::corresponding_accelerator_bitstreams.find(
          query_node.first);
  if (bitstreams_iterator !=
      query_scheduling_data::corresponding_accelerator_bitstreams.end()) {
    return bitstreams_iterator->second;
  } else {
    throw std::runtime_error("Unsopported set of modules!");
  }
}

auto QueryManager::GetModuleCountFromQueryNode(
    const std::pair<query_scheduling_data::ConfigurableModuleSet,
                    std::vector<query_scheduling_data::QueryNode>>& query_node)
    -> int {
  int module_count = 1;  // +1 for DMA module
  for (const auto& operation_module : query_node.first) {
    module_count += operation_module.second;
  }
  return module_count;
}

void QueryManager::RunQueries(
    std::vector<query_scheduling_data::QueryNode> starting_query_nodes) {
  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");
  MemoryManager memory_manager;
  FPGAManager fpga_manager(&memory_manager);

  std::queue<std::pair<query_scheduling_data::ConfigurableModuleSet,
                       std::vector<query_scheduling_data::QueryNode>>>
      accelerated_query_node_sets;

  NodeScheduler::FindAcceleratedQueryNodeSets(&accelerated_query_node_sets,
                                              std::move(starting_query_nodes));

  while (!accelerated_query_node_sets.empty()) {
    const auto executable_query_node = accelerated_query_node_sets.front();

    // Load the bitstream
    memory_manager.LoadBitstream(
        GetBitstreamFileFromQueryNode(executable_query_node),
        GetModuleCountFromQueryNode(executable_query_node) *
            query_acceleration_constants::kModuleSize);

    for (const auto& current_node : executable_query_node.second) {
      // These ID allocations need to be improved
      std::vector<int> input_stream_id_vector;
      std::vector<std::unique_ptr<MemoryBlockInterface>>
          allocated_input_memory_blocks;
      for (int stream_id = 0;
           stream_id < current_node.input_data_definition_files.size();
           stream_id++) {
        input_stream_id_vector.push_back(stream_id);
        allocated_input_memory_blocks.push_back(
            memory_manager.GetAvailableMemoryBlock());
      }
      std::vector<int> output_stream_id_vector;
      std::vector<std::unique_ptr<MemoryBlockInterface>>
          allocated_output_memory_blocks;
      for (int stream_id = 0;
           stream_id < current_node.output_data_definition_files.size();
           stream_id++) {
        output_stream_id_vector.push_back(stream_id);
        allocated_output_memory_blocks.push_back(
            memory_manager.GetAvailableMemoryBlock());
      }

      std::map<StreamDataParameters, std::unique_ptr<MemoryBlockInterface>>
          input_stream_parameters_map;
      std::map<StreamDataParameters, std::unique_ptr<MemoryBlockInterface>>
          output_stream_parameters_map;

      TableManager::ReadInputTables(&input_stream_parameters_map, data_manager,
                                    current_node.input_data_definition_files,
                                    input_stream_id_vector,
                                    allocated_input_memory_blocks);

      std::vector<TableData> expected_output_tables(16);
      TableManager::ReadExpectedTables(
          &output_stream_parameters_map, data_manager,
          current_node.output_data_definition_files, output_stream_id_vector,
          allocated_output_memory_blocks, expected_output_tables);

      std::vector<StreamDataParameters> input_stream_parameters_list;
      for (auto const& [parameters, memory_block] :
           input_stream_parameters_map) {
        input_stream_parameters_list.push_back(parameters);
      }
      std::vector<StreamDataParameters> output_stream_parameters_list;
      for (auto const& [parameters, memory_block] :
           output_stream_parameters_map) {
        output_stream_parameters_list.push_back(parameters);
      }

      // Multiple operations can now be run simultaneously - Needs reworking!
      fpga_manager.SetupQueryAcceleration(
          {{input_stream_parameters_list, output_stream_parameters_list,
            current_node.operation_type}});

      std::cout << "Running query!" << std::endl;
      auto result_sizes = fpga_manager.RunQueryAcceleration();
      std::cout << "Query done!" << std::endl;

      for (auto it = input_stream_parameters_map.begin();
           it != input_stream_parameters_map.end(); it++) {
        memory_manager.FreeMemoryBlock(std::move(it->second));
      }
      // Delete all for now.
      input_stream_parameters_map.clear();

      std::vector<TableData> output_tables = expected_output_tables;
      TableManager::ReadResultTables(&output_stream_parameters_map,
                                     output_tables, result_sizes);

      for (auto it = output_stream_parameters_map.begin();
           it != output_stream_parameters_map.end(); it++) {
        memory_manager.FreeMemoryBlock(std::move(it->second));
      }
      // Delete all for now.
      output_stream_parameters_map.clear();

      for (auto const& output_stream_id : output_stream_id_vector) {
        std::cout << "Result has " << result_sizes[output_stream_id] << " rows!"
                  << std::endl;

        CheckTableData(expected_output_tables[output_stream_id],
                       output_tables[output_stream_id]);
      }
    }

    accelerated_query_node_sets.pop();
  }
}
