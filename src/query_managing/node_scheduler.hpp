#pragma once
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"

namespace dbmstodspi {
namespace query_managing {

/**
 * @brief Class to schedule nodes to groups of different FPGA runs.
 */
class NodeScheduler {
 public:
  /**
   * @brief Find groups of accelerated query nodes which can be run in the same
   * FPGA run.
   * @param accelerated_query_node_sets Queue of groups of accelerated query
   * nodes to be accelerated next.
   * @param starting_nodes Input vector of leaf nodes from which the parsing can
   * begin.
   * @param supported_accelerator_bitstreams Map of hardware module combinations
   * which have a corresponding bitstream.
   */
  static void FindAcceleratedQueryNodeSets(
      std::queue<std::pair<query_scheduling_data::ConfigurableModulesVector,
                           std::vector<query_scheduling_data::QueryNode>>>
          *accelerated_query_node_sets,
      std::vector<query_scheduling_data::QueryNode> starting_nodes,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams);

 private:
  static void RemoveLinkedNodes(
      std::vector<query_scheduling_data::QueryNode *> &linked_nodes,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes);
  static auto IsModuleSetSupported(
      query_scheduling_data::ConfigurableModulesVector module_set,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams) -> bool;
  static auto IsNodeIncluded(
      std::vector<query_scheduling_data::QueryNode> node_vector,
      query_scheduling_data::QueryNode searched_node) -> bool;
  static auto IsNodeAvailable(
      std::vector<query_scheduling_data::QueryNode> scheduled_nodes,
      query_scheduling_data::QueryNode current_node) -> bool;
  static auto FindNextAvailableNode(
      std::vector<query_scheduling_data::QueryNode> &already_scheduled_nodes,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes)
      -> std::vector<query_scheduling_data::QueryNode>::iterator;
  static auto FindMinPosition(
      query_scheduling_data::QueryNode &current_node,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      query_scheduling_data::ConfigurableModulesVector &current_modules_vector)
      -> int;
  static void CheckNodeForModuleSet(
      std::vector<query_scheduling_data::QueryNode>::iterator &iterator,
      query_scheduling_data::ConfigurableModulesVector &current_set,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      std::vector<query_scheduling_data::QueryNode> &scheduled_queries,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams);
  static auto FindSuitableModulePosition(
      query_scheduling_data::QueryNode &current_node,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      query_scheduling_data::ConfigurableModulesVector &current_modules_vector,
      const std::map<query_scheduling_data::ConfigurableModulesVector,
                     std::string> &supported_accelerator_bitstreams) -> int;
  static auto CreateNewModulesVector(
      fpga_managing::operation_types::QueryOperation query_operation,
      int current_position,
      query_scheduling_data::ConfigurableModulesVector current_modules_vector)
      -> query_scheduling_data::ConfigurableModulesVector;
};

}  // namespace query_managing
}  // namespace dbmstodspi