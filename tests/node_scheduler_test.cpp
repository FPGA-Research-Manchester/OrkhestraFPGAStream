#include "node_scheduler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
/// Good documentation for using matchers:
/// https://github.com/google/googletest/blob/master/docs/reference/matchers.md
namespace {

const int kSomeInteger = 2;

using ModulesCombo = dbmstodspi::query_managing::query_scheduling_data::
    ConfigurableModulesVector;
using Node = dbmstodspi::query_managing::query_scheduling_data::QueryNode;
using OpType = dbmstodspi::fpga_managing::operation_types::QueryOperationType;

void CheckNodeFields(
    std::vector<std::string> input_files, std::vector<std::string> output_files,
    OpType operation, std::vector<Node*> next, std::vector<Node*> previous,
    dbmstodspi::query_managing::query_scheduling_data::NodeOperationParameters
        parameters,
    int location, Node comparable_node) {
  EXPECT_THAT(comparable_node,
              testing::Field("input_files", &Node::input_data_definition_files,
                             input_files));
  EXPECT_THAT(
      comparable_node,
      testing::Field("output_files", &Node::output_data_definition_files,
                     output_files));
  EXPECT_THAT(comparable_node,
              testing::Field("operation", &Node::operation_type, operation));
  EXPECT_THAT(comparable_node, testing::Field("next", &Node::next_nodes, next));
  EXPECT_THAT(comparable_node,
              testing::Field("previous", &Node::previous_nodes, previous));
  EXPECT_THAT(
      comparable_node,
      testing::Field("parameters", &Node::operation_parameters, parameters));
  EXPECT_THAT(comparable_node,
              testing::Field("location", &Node::module_location, location));
}

void CheckNodeEquality(Node real_node, Node expected_node) {
  CheckNodeFields(expected_node.input_data_definition_files,
                  expected_node.output_data_definition_files,
                  expected_node.operation_type, expected_node.next_nodes,
                  expected_node.previous_nodes,
                  expected_node.operation_parameters,
                  expected_node.module_location, real_node);
}

TEST(NodeSchedulerTest, MultipleAvailableNodesFindsCorrectNode) {
  Node filtering_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kFilter,    {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  auto expected_node = filtering_query;
  std::vector<Node> starting_nodes = {filtering_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  expected_node.module_location = 1;

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kFilter, {}}}, {expected_node}});
  ASSERT_EQ(expected_results, scheduling_results);

  // This should be used more extensively in the future to check all nodes.
  CheckNodeEquality(scheduling_results.front().second[0], expected_node);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithDifferentRuns) {
  Node first_query = {{"Some_input.csv"},  {"Some_output.csv"},
                      OpType::kLinearSort, {nullptr},
                      {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"},  {"Some_output.csv"},
                       OpType::kLinearSort, {nullptr},
                       {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  std::vector<Node> starting_nodes = {first_query, second_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kLinearSort, {}}}, {first_query}});
  expected_results.push({{{OpType::kLinearSort, {}}}, {second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoNodesWereFoundWithinTheSameRun) {
  Node first_query = {{"Some_input.csv"},  {"Some_output.csv"},
                      OpType::kLinearSort, {nullptr},
                      {nullptr},           {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"}, {"Some_output.csv"},
                       OpType::kMergeSort, {nullptr},
                       {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}, {OpType::kMergeSort, {}}},
       "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  std::vector<Node> starting_nodes = {first_query, second_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kLinearSort, {}}, {OpType::kMergeSort, {}}},
                         {first_query, second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, PassthroughNodeIsUsedInTheSameRun) {
  Node first_query = {{"Some_input.csv"},
                      {"Some_output.csv"},
                      OpType::kPassThrough,
                      {nullptr},
                      {nullptr},
                      {{{}}, {{}, {kSomeInteger}}, {{}}}};
  Node second_query = {{"Some_input.csv"}, {"Some_output.csv"},
                       OpType::kMergeSort, {nullptr},
                       {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{}}}};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  std::vector<Node> starting_nodes = {first_query, second_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push(
      {{{OpType::kMergeSort, {}}}, {second_query, first_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithDifferentRuns) {
  Node first_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kJoin,      {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}};
  Node second_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kMergeSort, {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}};
  first_query.next_nodes = {&second_query};
  second_query.previous_nodes = {&first_query};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  std::vector<Node> starting_nodes = {first_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  auto expected_first_query = first_query;
  auto expected_second_query = second_query;
  expected_first_query.next_nodes = {nullptr};
  expected_second_query.previous_nodes = {nullptr};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kJoin, {}}}, {expected_first_query}});
  expected_results.push({{{OpType::kMergeSort, {}}}, {expected_second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

TEST(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithinTheSameRun) {
  Node first_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kJoin,      {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}};
  Node second_query = {
      {"Some_input.csv"}, {"Some_output.csv"},
      OpType::kMergeSort, {nullptr},
      {nullptr},          {{{}}, {{}, {kSomeInteger}}, {{kSomeInteger}}}};
  first_query.next_nodes = {&second_query};
  second_query.previous_nodes = {&first_query};

  const std::map<ModulesCombo, std::string> supported_bitstreams = {
      {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}}, "Some_bitstream"},
      {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
      {{{OpType::kFilter, {}}}, "Some_bitstream"},
      {{{OpType::kPassThrough, {}}}, "Some_bitstream"},
      {{{OpType::kJoin, {}}, {OpType::kMergeSort, {}}}, "Some_bitstream"}};

  const std::map<OpType, std::vector<std::vector<int>>> existing_modules = {
      {OpType::kMergeSort, {}},
      {OpType::kJoin, {}},
      {OpType::kLinearSort, {}},
      {OpType::kFilter, {}},
      {OpType::kPassThrough, {}}};

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> scheduling_results;
  std::vector<Node> starting_nodes = {first_query};

  dbmstodspi::query_managing::NodeScheduler::FindAcceleratedQueryNodeSets(
      &scheduling_results, starting_nodes, supported_bitstreams,
      existing_modules);

  std::queue<std::pair<ModulesCombo, std::vector<Node>>> expected_results;
  expected_results.push({{{OpType::kJoin, {}}, {OpType::kMergeSort, {}}},
                         {first_query, second_query}});
  ASSERT_EQ(expected_results, scheduling_results);
}

}  // namespace