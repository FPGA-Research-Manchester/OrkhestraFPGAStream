#pragma once
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"

namespace dbmstodspi::query_managing {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs.
 */
class NodeScheduler {
 public:
  /**
   * @brief Find groups of accelerated query nodes which can be run in the same
   * FPGA run.
   * @param accelerated_query_node_runs Queue of groups of accelerated query
   * nodes to be accelerated next.
   * @param starting_nodes Input vector of leaf nodes from which the parsing can
   * begin.
   * @param supported_accelerator_bitstreams Map of hardware module combinations
   * which have a corresponding bitstream.
   * @param existing_modules_library Map of hardware modules and the available
   * variations with different computational capacity values.
   */
  static void FindAcceleratedQueryNodeSets(
      std::queue<std::pair<query_scheduling_data::ConfigurableModulesVector,
                           std::vector<query_scheduling_data::QueryNode>>>
          *accelerated_query_node_runs,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams,
      const std::map<fpga_managing::operation_types::QueryOperationType,
                     std::vector<std::vector<int>>> &existing_modules_library);

 private:
  static void RemoveLinkedNodes(
      std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
          &linked_nodes,
      const std::vector<query_scheduling_data::QueryNode> &current_query_nodes);
  static auto IsModuleSetSupported(
      const query_scheduling_data::ConfigurableModulesVector &module_set,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams) -> bool;
  static auto IsNodeIncluded(
      const std::vector<query_scheduling_data::QueryNode> &node_vector,
      const query_scheduling_data::QueryNode &searched_node) -> bool;
  static auto IsNodeAvailable(
      const std::vector<query_scheduling_data::QueryNode> &scheduled_nodes,
      const query_scheduling_data::QueryNode &current_node) -> bool;
  /*static auto FindNextAvailableNode(
      const std::vector<query_scheduling_data::QueryNode>
          &already_scheduled_nodes,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes)
      -> std::vector<query_scheduling_data::QueryNode>::iterator &;*/
  static auto FindMinPosition(
      const query_scheduling_data::QueryNode *current_node,
      const std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      const query_scheduling_data::ConfigurableModulesVector
          &current_modules_vector) -> int;
  static void CheckNodeForModuleSet(
      int node_index,
      query_scheduling_data::ConfigurableModulesVector &current_modules_vector,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      std::vector<query_scheduling_data::QueryNode> &scheduled_queries,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams,
      const std::map<fpga_managing::operation_types::QueryOperationType,
                     std::vector<std::vector<int>>> &existing_modules_library);
  static auto FindSuitableModuleCombination(
      query_scheduling_data::QueryNode *current_node,
      const std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      const query_scheduling_data::ConfigurableModulesVector
          &current_modules_vector,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams,
      const std::map<fpga_managing::operation_types::QueryOperationType,
                     std::vector<std::vector<int>>> &existing_modules_library)
      -> query_scheduling_data::ConfigurableModulesVector;
  static auto CreateNewModulesVector(
      fpga_managing::operation_types::QueryOperationType query_operation,
      int current_position,
      const query_scheduling_data::ConfigurableModulesVector
          &current_modules_vector,
      const std::vector<int> &module_parameters)
      -> query_scheduling_data::ConfigurableModulesVector;
  static auto CheckModuleParameterSupport(
      std::vector<int> module_parameters,
      const query_scheduling_data::ConfigurableModulesVector
          &current_modules_vector,
      int module_position, int parameter_option_index,
      fpga_managing::operation_types::QueryOperationType query_operation,
      std::vector<std::vector<int>> current_module_possible_parameters,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams)
      -> query_scheduling_data::ConfigurableModulesVector;
};

}  // namespace dbmstodspi::query_managing